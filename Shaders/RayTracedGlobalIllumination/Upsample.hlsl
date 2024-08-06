#define SET 0

[[vk::binding(0, SET)]] Texture2D<float4> g_input        : register(t0);
[[vk::binding(1, SET)]] Texture2D<float>  g_depth        : register(t1);
[[vk::binding(2, SET)]] Texture2D<float>  g_depth_lowres : register(t2);

// Outputs
[[vk::binding(3, SET)]] RWTexture2D<float4>  g_output     : register(u0);

[[vk::push_constant]]
struct _Params {
    int2 OutputSize;
    int DownsampleFactor;
} Params;

#define NORMAL_SIGMA 32.0
#define DEPTH_SIGMA 4096.0

float GetEdgeStoppingDepthWeight(float center_depth, float neighbor_depth)
{
    return exp(-abs(center_depth - neighbor_depth) * center_depth * DEPTH_SIGMA);
}

min16float GetEdgeStoppingNormalWeight(min16float3 normal_p, min16float3 normal_q)
{
    return pow(saturate(dot(normal_p, normal_q)), NORMAL_SIGMA);
}

[numthreads(8, 8, 1)]
void main(int2 dtid : SV_DispatchThreadID)
{
    if (any(dtid >= Params.OutputSize))
        return;

    // closest lowres pixel
    int2 InputPixel = int2(round(float2(dtid) / float(Params.DownsampleFactor)));

#if 1
    float3 Result = float3(0, 0, 0);
    float WeightSum = 0.0f;

    float ReferenceDepth = g_depth[dtid].x;
    int Samples = Params.DownsampleFactor - 1;
    Samples = 1; // 3x3 seems to work just fine

    for (int i = -Samples; i <= Samples; i++)
    {
        for (int j = -Samples; j <= Samples; j++)
        {
            int2 Pos = InputPixel + int2(i, j);

            float3 Sample = g_input[Pos].rgb;
            float  SampleLowresDepth = g_depth_lowres[Pos].x;

            float Weight = GetEdgeStoppingDepthWeight(ReferenceDepth, SampleLowresDepth);

            Result += Sample * Weight;
            WeightSum += Weight;
        }
    }

    Result /= WeightSum;
    g_output[dtid] = float4(Result, 1);

#else
    // nearest
    g_output[dtid] = g_input[InputPixel];
#endif
}