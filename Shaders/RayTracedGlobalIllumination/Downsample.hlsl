#define SET 0
#include "../Common.hlsli"

// Inputs
[[vk::binding(0, SET)]] Texture2D<float>  g_depth_buffer  : register(t0);
[[vk::binding(1, SET)]] Texture2D<float2> g_normals       : register(t1);
[[vk::binding(2, SET)]] Texture2D<float>  g_depth_history : register(t2);
[[vk::binding(3, SET)]] Texture2D<float2> g_velocity      : register(t3);

// Outputs
[[vk::binding(4, SET)]] [[vk::image_format("r32f")]]  RWTexture2D<float>       g_output_depth         : register(u0);
[[vk::binding(5, SET)]] [[vk::image_format("rg16f")]] RWTexture2D<float2>      g_output_normals       : register(u1);
[[vk::binding(6, SET)]] [[vk::image_format("r32f")]]  RWTexture2D<float>       g_output_depth_history : register(u2);
[[vk::binding(7, SET)]] [[vk::image_format("rg16f")]] RWTexture2D<float2>      g_output_velocity      : register(u3);
[[vk::binding(8, SET)]] [[vk::image_format("rgba32f")]] RWTexture2D<float4>    g_output_source_pixel  : register(u4);

[[vk::push_constant]]
struct _Params {
    int2 Size;
    int2 SourceSize;
    int DownsampleFactor;
} Params;

[numthreads(8, 8, 1)]
void main(int2 dtid : SV_DispatchThreadID)
{
    if (any(dtid >= Params.Size))
        return;

    float  ResultDepth = DEVICE_DEPTH_FAR;
    float  DepthHistory = DEVICE_DEPTH_FAR;
    float2 ResultNormal = float2(0.5, 0.5);
    float2 ResultVelocity = float2(0,0);

    const int2 SourcePosStart = dtid * Params.DownsampleFactor;
    float2 ResultSourcePixel = min(SourcePosStart, Params.SourceSize - 1);

    for (int i = 0; i < Params.DownsampleFactor; i++)
    {
        for (int j = 0; j < Params.DownsampleFactor; j++)
        {
            const int2 SourcePos = min(SourcePosStart + int2(i, j), Params.SourceSize - 1);
            const float CandidateDepth = g_depth_buffer[SourcePos].x;

            if (CandidateDepth > ResultDepth)
            {
                ResultDepth = CandidateDepth;
                DepthHistory = g_depth_history[SourcePos].x;
                ResultNormal = g_normals[SourcePos].xy;
                ResultVelocity = g_velocity[SourcePos].xy;
                ResultSourcePixel = SourcePos;
            }
        }
    }

    g_output_depth[dtid] = ResultDepth;
    g_output_normals[dtid] = ResultNormal;
    g_output_depth_history[dtid] = DepthHistory;
    g_output_velocity[dtid] = ResultVelocity;
    g_output_source_pixel[dtid] = float4(ResultSourcePixel, 0.0, 0.0);
}
