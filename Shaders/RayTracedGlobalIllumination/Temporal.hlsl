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

// Misc
[[vk::binding(7, SET)]] SamplerState g_sampler            : register(s7);

[[vk::push_constant]]
struct _Params {
    float4x4 ProjectionInv;
    // float4x4 ReprojectionMatrix;
    float4x4 ViewProjectionInv;
    float4x4 PrevViewProjection;
	int2 Size;
    int MaxSamples;
} Params;

float GetLinearDepth(int2 dtid, float depth)
{
    const float2 uv = (dtid + 0.5f) / float2(Params.Size);
	const float2 ndc = 2.0f * float2(uv.x, 1.0f - uv.y) - 1.0f;

    float4 Projected = mul(Params.ProjectionInv, float4(ndc, depth, 1));
    return abs(Projected.z / Projected.w);
}

#define CLAMP_SCREEN_COORDS(a) clamp(a, int2(0,0), Params.Size - 1)

[numthreads(8, 8, 1)]
void main(int2 dtid : SV_DispatchThreadID)
{
    if (any(dtid >= Params.Size))
        return;

    const float2 VelocityRaw = g_velocity[dtid] / 2;
    const float2 Uv = (dtid + 0.5f) / float2(Params.Size);
    const float2 UvReprojected = Uv - VelocityRaw;

    float2 Velocity = floor(g_velocity[dtid] / 2 * Params.Size + 0.5);
    int2 PrevCoords = int2(dtid - Velocity);
    int2 PrevCoordsClamped = CLAMP_SCREEN_COORDS(PrevCoords);

    float SampleCount = g_sample_count[PrevCoordsClamped];

    float3 Debug = float3(0,0,0);

    // disocclusion
    if (any(PrevCoords < 0) || any(PrevCoords > Params.Size - 1))
    {
        SampleCount = 0;
    } else
    {
        const float2 uv = (dtid + 0.5f) / float2(Params.Size);
	    const float2 ndc = 2.0f * float2(uv.x, 1.0f - uv.y) - 1.0f;

        float Depth = g_depth[dtid];

        float4 WorldSpace = mul(Params.ViewProjectionInv, float4(ndc, Depth, 1.0f));
        WorldSpace /= WorldSpace.w;
        float4 PrevNdc = mul(Params.PrevViewProjection, WorldSpace);
        PrevNdc /= PrevNdc.w;

        #if 0
        float2 PrevUv = (PrevNdc.xy+1)/2;
        PrevUv.y = 1 - PrevUv.y;
        int2 ReprojectedCoords = CLAMP_SCREEN_COORDS(PrevUv * Params.Size);
        PrevCoordsClamped = ReprojectedCoords;
        PrevCoords = ReprojectedCoords;
        Debug = PrevNdc.xyz;
        #endif

        float ReprojectedLinearDepth = GetLinearDepth(dtid, PrevNdc.z);
        float PrevLinearDepth = GetLinearDepth(PrevCoords, g_depth_history[PrevCoords]);
        
        // distance-aware depth rejection
        const float DepthDifference = abs(PrevLinearDepth - ReprojectedLinearDepth) / ReprojectedLinearDepth;
        const float DepthThreshold = max(1e-2f, 0.05f * ReprojectedLinearDepth);

        if (DepthDifference >= DepthThreshold)
        {
            SampleCount = 0;
        }
        else
        {
            SampleCount += 1;
        }
    }

    SampleCount = clamp(SampleCount, 0, Params.MaxSamples);

    //float3 HistoryValue = g_history[PrevCoordsClamped];
    float3 HistoryValue = g_history.SampleLevel(g_sampler, UvReprojected, 0);

    // neighbourhood clamp
    if (false)
    {
        float3 MinValue = float3(9999, 9999, 9999);
        float3 MaxValue = float3(-9999, -9999, -9999);

        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                float3 Value = g_input[CLAMP_SCREEN_COORDS(dtid + int2(i,j))];
                MinValue = min(MinValue, Value);
                MaxValue = max(MaxValue, Value);
            }
        }

        HistoryValue = clamp(HistoryValue, MinValue, MaxValue);
    }

    float HistoryWeight = 1 / (SampleCount + 1);
    float3 Result = lerp(HistoryValue, g_input[dtid], HistoryWeight);
    g_output[dtid] = float4(Result, 1);
    // g_output[dtid] = float4(Debug, 1);
    g_sample_count[dtid] = SampleCount;
}