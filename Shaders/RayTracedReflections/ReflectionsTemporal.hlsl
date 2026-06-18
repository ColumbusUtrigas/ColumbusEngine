#pragma pack_matrix(row_major)

#include "../Common.hlsli"
#define GPU_SCENE_NO_BINDINGS
#include "../GPUScene.hlsli"

[[vk::binding(0, 0)]]   Texture2D<float4> InCurrent;
[[vk::binding(1, 0)]]   Texture2D<float4> InHistory;
[[vk::binding(2, 0)]]   Texture2D<float2> InVelocity;
[[vk::binding(3, 0)]]   Texture2D<float>  InDepth;
[[vk::binding(4, 0)]]   Texture2D<float>  InDepthHistory;
[[vk::binding(5, 0)]]   Texture2D<float>  InHistorySampleCount;
[[vk::binding(6, 0)]]   Texture2D<float4> InRays;
[[vk::binding(7, 0)]]   Texture2D<float>  InResolvedHitDistance;
[[vk::binding(8, 0)]]   Texture2D<float2> InNormal;
[[vk::binding(9, 0)]]   Texture2D<float2> InRoughnessMetallic;
[[vk::binding(10, 0)]]  Texture2D<float2> InNormalHistory;
[[vk::binding(11, 0)]]  Texture2D<float2> InRoughnessMetallicHistory;
[[vk::binding(12, 0)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> OutRadiance;
[[vk::binding(13, 0)]] [[vk::image_format("r16f")]]    RWTexture2D<float>  OutSampleCount;
[[vk::binding(14, 0)]]  SamplerState      LinearSampler;
[[vk::binding(15, 0)]]  StructuredBuffer<GPUSceneStruct> GPUSceneScene;

[[vk::push_constant]]
struct _Params
{
    int2 Size;
    int MaxSamples;
    int _Padding;
} Params;

#define CLAMP_SCREEN_COORDS(a) clamp(a, int2(0, 0), Params.Size - 1)

float3 RGBToYCoCg(float3 c)
{
    return float3(
        c.x * 0.25 + c.y * 0.5 + c.z * 0.25,
        c.x * 0.5 - c.z * 0.5,
        -c.x * 0.25 + c.y * 0.5 - c.z * 0.25);
}

float3 YCoCgToRGB(float3 c)
{
    return float3(
        c.x + c.y - c.z,
        c.x + c.z,
        c.x - c.y - c.z);
}

float GetLinearDepth(int2 pixel, float depth)
{
    const float2 uv = (pixel + 0.5f) / float2(Params.Size);
    const float2 ndc = ScreenUVToNDC(uv);

    float4 projected = mul(GPUSceneScene[0].CameraCur.InverseProjectionMatrix, float4(ndc, depth, 1));
    return abs(projected.z / projected.w);
}

bool ValidatePreviousSurface(float2 uvReprojected, int2 prevCoordsClamped, float reprojectedDepth, float3 currentNormal, float currentRoughness, bool validateMaterial)
{
    if (!all(uvReprojected >= 0.0) || !all(uvReprojected < 1.0))
        return false;

    float prevDepth = InDepthHistory[prevCoordsClamped];
    if (IsSkyDepth(prevDepth))
        return false;

    float reprojectedLinearDepth = GetLinearDepth(prevCoordsClamped, reprojectedDepth);
    float prevLinearDepth = GetLinearDepth(prevCoordsClamped, prevDepth);
    float depthDifference = abs(prevLinearDepth - reprojectedLinearDepth) / max(reprojectedLinearDepth, 1e-3f);
    if (depthDifference >= 0.05f)
        return false;

    if (!validateMaterial)
        return true;

    float3 prevNormal = NormalDecode(InNormalHistory.SampleLevel(LinearSampler, uvReprojected, 0).xy);
    float prevRoughness = InRoughnessMetallicHistory.SampleLevel(LinearSampler, uvReprojected, 0).x;
    float normalThreshold = lerp(0.995, 0.90, saturate(currentRoughness));
    if (dot(currentNormal, prevNormal) < normalThreshold)
        return false;

    float roughnessThreshold = lerp(0.02, 0.20, saturate(currentRoughness));
    if (abs(prevRoughness - currentRoughness) > roughnessThreshold)
        return false;

    return true;
}

bool ValidatePreviousHit(float3 hitPosition, float3 currentNormal, float currentRoughness)
{
    float4 prevHitClip = mul(GPUSceneScene[0].CameraPrev.ViewProjectionMatrix, float4(hitPosition, 1.0));
    if (prevHitClip.w <= 1e-5)
        return false;

    float2 hitUv = ClipToScreenUV(prevHitClip);
    int2 hitPrevCoords = CLAMP_SCREEN_COORDS(int2(hitUv * float2(Params.Size)));
    return ValidatePreviousSurface(hitUv, hitPrevCoords, prevHitClip.z / prevHitClip.w, currentNormal, currentRoughness, false);
}

[numthreads(8, 8, 1)]
void main(int2 dtid : SV_DispatchThreadID)
{
    if (any(dtid >= Params.Size))
        return;

    const float depth = InDepth[dtid];
    const float4 current = InCurrent[dtid];

    if (IsSkyDepth(depth))
    {
        OutRadiance[dtid] = current;
        OutSampleCount[dtid] = 1.0;
        return;
    }

    const float3 currentNormal = NormalDecode(InNormal[dtid]);
    const float currentRoughness = saturate(InRoughnessMetallic[dtid].x);
    const float2 velocity = InVelocity[dtid] * 0.5f;
    const float2 uv = (dtid + 0.5f) / float2(Params.Size);
    const float2 surfaceUvReprojected = uv - velocity;
    const float2 velocityPixels = floor(velocity * float2(Params.Size) + 0.5);
    const int2 surfacePrevCoords = int2(dtid - velocityPixels);
    const int2 surfacePrevCoordsClamped = CLAMP_SCREEN_COORDS(surfacePrevCoords);

    bool historyValid = false;
    float2 uvReprojected = surfaceUvReprojected;
    int2 prevCoordsClamped = surfacePrevCoordsClamped;

    if (all(surfacePrevCoords >= 0) && all(surfacePrevCoords < Params.Size))
    {
        const float2 ndc = ScreenUVToNDC(uv);
        float4 worldSpace = mul(GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix, float4(ndc, depth, 1.0f));
        worldSpace /= worldSpace.w;

        float4 prevNdc = mul(GPUSceneScene[0].CameraPrev.ViewProjectionMatrix, worldSpace);
        prevNdc /= prevNdc.w;

        historyValid = ValidatePreviousSurface(surfaceUvReprojected, surfacePrevCoordsClamped, prevNdc.z, currentNormal, currentRoughness, true);
        if (historyValid)
        {
            float hitDistance = InResolvedHitDistance[dtid];
            float4 rayPacked = InRays[dtid];
            if (hitDistance > 0.0 && rayPacked.w != 0.0 && dot(rayPacked.xyz, rayPacked.xyz) > 1e-6)
            {
                float3 worldPosition = ReconstructWorldPositionFromDepth(uint2(dtid), depth, GPUSceneScene[0].RenderSize, GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix);
                float3 hitPosition = worldPosition + normalize(rayPacked.xyz) * hitDistance;
                historyValid = ValidatePreviousHit(hitPosition, currentNormal, currentRoughness);
            }
        }
    }

    float historySampleCount = historyValid ? InHistorySampleCount[prevCoordsClamped] : 0.0;
    if (!isfinite(historySampleCount))
        historySampleCount = 0.0;
    historySampleCount = clamp(historySampleCount, 0.0, (float)Params.MaxSamples);

    float3 history = historyValid ? InHistory.SampleLevel(LinearSampler, uvReprojected, 0).rgb : current.rgb;

    float3 minValue = float3(9999.0, 9999.0, 9999.0);
    float3 maxValue = float3(-9999.0, -9999.0, -9999.0);
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float3 value = RGBToYCoCg(InCurrent[CLAMP_SCREEN_COORDS(dtid + int2(x, y))].rgb);
            minValue = min(minValue, value);
            maxValue = max(maxValue, value);
        }
    }
    history = YCoCgToRGB(clamp(RGBToYCoCg(history), minValue, maxValue));

    float samples = historyValid ? min(historySampleCount + 1.0, (float)Params.MaxSamples) : 1.0;
    samples = min(samples, lerp(2.0, (float)Params.MaxSamples, saturate(currentRoughness / 0.05)));
    float alpha = 1.0 / max(samples, 1.0);
    float3 result = lerp(history, current.rgb, alpha);

    OutRadiance[dtid] = float4(result, current.a);
    OutSampleCount[dtid] = samples;
}
