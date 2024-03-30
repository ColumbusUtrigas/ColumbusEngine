#pragma pack_matrix(row_major)

#define SET 0

// Inputs
[[vk::binding(0, SET)]] Texture2D<float2> g_velocity      : register(t0);
[[vk::binding(1, SET)]] Texture2D<float3> g_input         : register(t1);
[[vk::binding(2, SET)]] Texture2D<float3> g_history       : register(t2);
[[vk::binding(3, SET)]] Texture2D<float>  g_depth         : register(t3);
[[vk::binding(4, SET)]] Texture2D<float>  g_depth_history : register(t4);

// RW
[[vk::binding(5, SET)]] RWTexture2D<float> g_sample_count : register(u0);

// Outputs
[[vk::binding(6, SET)]] RWTexture2D<float4> g_output      : register(u1);

[[vk::push_constant]]
struct _Params {
    float4x4 ProjectionInv;
    float4x4 ReprojectionMatrix;
	int2 Size;
} Params;

float GetLinearDepth(int2 dtid, float depth)
{
    const float2 uv = (dtid + 0.5f) / float2(Params.Size);
	const float2 ndc = 2.0f * float2(uv.x, 1.0f - uv.y) - 1.0f;

    float4 Projected = mul(Params.ProjectionInv, float4(ndc, depth, 1));
    return abs(Projected.z / Projected.w);
}

[numthreads(8, 8, 1)]
void main(int2 dtid : SV_DispatchThreadID)
{
    if (any(dtid >= Params.Size))
        return;

    float2 Velocity = g_velocity[dtid] / 2 * Params.Size;
    int2 PrevCoords = int2(dtid - Velocity);
    int2 PrevCoordsClamped = clamp(PrevCoords, int2(0,0), Params.Size - 1);

    float SampleCount = g_sample_count[PrevCoordsClamped];

    // disocclusion
    if (any(PrevCoords < 0) || any(PrevCoords > Params.Size - 1))
    {
        SampleCount = 0;
    } else
    {
        const float2 uv = (dtid + 0.5f) / float2(Params.Size);
	    const float2 ndc = 2.0f * float2(uv.x, 1.0f - uv.y) - 1.0f;
        // const float2 prevuv = (PrevCoords + 0.5f) / float2(Params.Size);

        float Depth = g_depth[dtid];

        // TODO: proper reprojection
        float4 ClipSpace = mul(Params.ReprojectionMatrix, float4(ndc, Depth, 1.0f));
		ClipSpace /= ClipSpace.w; // perspective divide

        float LinearDepth = GetLinearDepth(dtid, g_depth[dtid]);
        // float LinearDepth = GetLinearDepth(dtid, ClipSpace.z);
        float PrevLinearDepth = GetLinearDepth(PrevCoords, g_depth_history[PrevCoords]);

        const float DepthDifference = abs(PrevLinearDepth - LinearDepth) / LinearDepth;

        if (DepthDifference >= 1e-2f)
        {
            SampleCount = 0;
        }
        else
        {
            SampleCount += 1;
        }
    }

    SampleCount = clamp(SampleCount, 0, 100);

    float HistoryWeight = 1 / (SampleCount + 1);
    float3 Result = lerp(g_history[PrevCoordsClamped], g_input[dtid], HistoryWeight);
    g_output[dtid] = float4(Result, 1);
    g_sample_count[dtid] = SampleCount;
}