#pragma pack_matrix(row_major)

[[vk::binding(0, 0)]]   Texture2D<float4> InCurrent;
[[vk::binding(1, 0)]]   Texture2D<float4> InHistory;
[[vk::binding(2, 0)]]   Texture2D<float2> InVelocity;
[[vk::binding(3, 0)]]   Texture2D<float>  InDepth;
[[vk::binding(4, 0)]]   Texture2D<float>  InDepthHistory;
[[vk::binding(5, 0)]]   Texture2D<float>  InHistorySampleCount;
[[vk::binding(6, 0)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> OutRadiance;
[[vk::binding(7, 0)]] [[vk::image_format("r16f")]]    RWTexture2D<float>  OutSampleCount;
[[vk::binding(8, 0)]]   SamplerState      LinearSampler;

#include "../Common.hlsli"

[[vk::push_constant]]
struct _Params
{
    float4x4 ProjectionInv;
    float4x4 ViewProjectionInv;
    float4x4 PrevViewProjection;
    int2 Size;
    int MaxSamples;
    int _Padding;
} Params;

#define CLAMP_SCREEN_COORDS(a) clamp(a, int2(0, 0), Params.Size - 1)

bool IsSkyDepth(float depth)
{
    return abs(depth) < EPSILON || abs(depth - 1.0) < EPSILON;
}

float GetLinearDepth(int2 pixel, float depth)
{
    const float2 uv = (pixel + 0.5f) / float2(Params.Size);
    const float2 ndc = 2.0f * float2(uv.x, 1.0f - uv.y) - 1.0f;

    float4 projected = mul(Params.ProjectionInv, float4(ndc, depth, 1));
    return abs(projected.z / projected.w);
}

[numthreads(8, 8, 1)]
void main(int2 dtid : SV_DispatchThreadID)
{
    if (any(dtid >= Params.Size))
        return;

    const float2 velocity = InVelocity[dtid] * 0.5f;
    const float2 uv = (dtid + 0.5f) / float2(Params.Size);
    const float2 uvReprojected = uv - velocity;

    int2 prevCoords = int2(uvReprojected * float2(Params.Size));
    int2 prevCoordsClamped = CLAMP_SCREEN_COORDS(prevCoords);

    bool historyValid = all(uvReprojected >= 0.0) && all(uvReprojected < 1.0);

    float historySampleCount = InHistorySampleCount[prevCoordsClamped];
    if (!isfinite(historySampleCount))
        historySampleCount = 0.0;
    historySampleCount = clamp(historySampleCount, 0.0, (float)Params.MaxSamples);

    if (historyValid)
    {
        float depth = InDepth[dtid];
        float prevDepth = InDepthHistory[prevCoordsClamped];

        if (IsSkyDepth(depth) || IsSkyDepth(prevDepth))
        {
            historyValid = false;
        }
        else
        {
            const float2 ndc = 2.0f * float2(uv.x, 1.0f - uv.y) - 1.0f;
            float4 worldSpace = mul(Params.ViewProjectionInv, float4(ndc, depth, 1.0f));
            worldSpace /= worldSpace.w;

            float4 prevNdc = mul(Params.PrevViewProjection, worldSpace);
            prevNdc /= prevNdc.w;

            float reprojectedLinearDepth = GetLinearDepth(dtid, prevNdc.z);
            float prevLinearDepth = GetLinearDepth(prevCoordsClamped, prevDepth);

            const float depthDifference = abs(prevLinearDepth - reprojectedLinearDepth) / max(reprojectedLinearDepth, 1e-3f);
            const float depthThreshold = max(1e-2f, 0.05f * reprojectedLinearDepth);
            historyValid = depthDifference < depthThreshold;
        }
    }

    float4 current = InCurrent[dtid];
    float3 history = historyValid ? InHistory.SampleLevel(LinearSampler, uvReprojected, 0).rgb : current.rgb;

    float3 minValue = float3(9999.0, 9999.0, 9999.0);
    float3 maxValue = float3(-9999.0, -9999.0, -9999.0);
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float3 value = InCurrent[CLAMP_SCREEN_COORDS(dtid + int2(x, y))].rgb;
            minValue = min(minValue, value);
            maxValue = max(maxValue, value);
        }
    }
    history = clamp(history, minValue, maxValue);

    float samples = historyValid ? min(historySampleCount + 1.0, (float)Params.MaxSamples) : 1.0;
    float alpha = 1.0 / max(samples, 1.0);
    float3 result = lerp(history, current.rgb, alpha);

    OutRadiance[dtid] = float4(result, current.a);
    OutSampleCount[dtid] = samples;
}
