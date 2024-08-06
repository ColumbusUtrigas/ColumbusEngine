#define SET 0

// Inputs
[[vk::binding( 0, SET)]] Texture2D<float>  g_depth_buffer : register(t0);
[[vk::binding( 1, SET)]] Texture2D<float3> g_normals      : register(t1);
[[vk::binding( 2, SET)]] Texture2D<float>  g_sample_count : register(t2);
[[vk::binding( 3, SET)]] Texture2D<float4> g_input        : register(t3);

// Outputs
[[vk::binding(4, SET)]] RWTexture2D<float4> g_output      : register(u0);

[[vk::push_constant]]
struct _Params {
	int2 Size;
    int StepSize;
    int MaxSamples;
    int DominationNumber;
} Params;

#define NORMAL_SIGMA 32.0
#define DEPTH_SIGMA 1024.0

float GetEdgeStoppingDepthWeight(float center_depth, float neighbor_depth)
{
    return exp(-abs(center_depth - neighbor_depth) * center_depth * DEPTH_SIGMA);
}

float GetEdgeStoppingNormalWeight(float3 normal_p, float3 normal_q)
{
    return pow(saturate(dot(normal_p, normal_q)), NORMAL_SIGMA);
}

static const float GaussKernel9[9] = { 0.0002, 0.0060, 0.0606, 0.2417, 0.3829, 0.2417, 0.0606, 0.0060, 0.0002 };
static const float GaussKernel5[5] = { 0.0614, 0.2448, 0.3877, 0.2448, 0.0614 };

[numthreads(8, 8, 1)]
void main(int2 dtid : SV_DispatchThreadID)
{
    if (any(dtid >= Params.Size))
        return;

    float3 Result = float3(0,0,0);
    float TotalWeight = 0;

    float CenterDepth = g_depth_buffer[dtid];
    float3 CenterNormal = g_normals[dtid];

    if (CenterDepth <= 0.0001 || CenterDepth >= 0.9999)
    {
        // sky
        g_output[dtid] = float4(0,0,0,1);
        return;
    }

    float SampleCount = g_sample_count[dtid];
    float DominationCoefficient = clamp(float(Params.DominationNumber) / (SampleCount + 1), 0, 1);
    float3 CenterValue = g_input[dtid].xyz;

    int KernelRange = 2; // 5x5
    // int KernelRange = 4; // 9x9

    for (int i = -KernelRange; i <= KernelRange; i++)
    {
        for (int j = -KernelRange; j <= KernelRange; j++)
        {
            int2 Step = int2(i, j) * Params.StepSize;
            int2 Coords = clamp(dtid + Step, int2(0,0), Params.Size - 1);

            float Weight = GaussKernel5[i+KernelRange] * GaussKernel5[j+KernelRange];

            float Depth = g_depth_buffer[Coords];
            Weight *= GetEdgeStoppingDepthWeight(CenterDepth, Depth);

            float3 Normal = g_normals[Coords];
            Weight *= GetEdgeStoppingNormalWeight(CenterNormal, Normal);

            float SkyWeight = (Depth >= 0.9999 || Depth <= 0.0001) ? 0 : 1;
            Weight *= SkyWeight;

            Result += g_input[Coords].xyz * Weight;
            TotalWeight += Weight;
        }
    }

    Result /= TotalWeight;

    g_output[dtid] = float4(lerp(CenterValue, Result, DominationCoefficient), 1);
    // g_output[dtid] = float4(1-TotalWeight, 0, 0, 1);
}