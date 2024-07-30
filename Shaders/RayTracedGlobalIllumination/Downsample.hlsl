#define SET 0

// Inputs
[[vk::binding(0, SET)]] Texture2D<float>  g_depth_buffer  : register(t0);
[[vk::binding(1, SET)]] Texture2D<float3> g_normals       : register(t1);
[[vk::binding(2, SET)]] Texture2D<float4> g_world_pos     : register(t2);
[[vk::binding(3, SET)]] Texture2D<float>  g_depth_history : register(t3);
[[vk::binding(4, SET)]] Texture2D<float4> g_velocity      : register(t4);

// Outputs
[[vk::binding(5, SET)]] RWTexture2D<float>  g_output_depth         : register(u0);
[[vk::binding(6, SET)]] RWTexture2D<float4> g_output_normals       : register(u1);
[[vk::binding(7, SET)]] RWTexture2D<float4> g_output_world_pos     : register(u2);
[[vk::binding(8, SET)]] RWTexture2D<float4> g_output_depth_history : register(u2);
[[vk::binding(9, SET)]] RWTexture2D<float4> g_output_velocity      : register(u2);

[[vk::push_constant]]
struct _Params {
    int2 Size;
    int DownsampleFactor;
} Params;

[numthreads(8, 8, 1)]
void main(int2 dtid : SV_DispatchThreadID)
{
    if (any(dtid >= Params.Size))
        return;

    float  ResultDepth = 1.0f;
    float  DepthHistory = 1.0f;
    float3 ResultNormal = float3(0,0,0);
    float3 ResultWorldPos = float3(0,0,0);
    float3 ResultVelocity = float3(0,0,0);

    const int2 SourcePosStart = dtid * Params.DownsampleFactor;

    for (int i = 0; i < Params.DownsampleFactor; i++)
    {
        for (int j = 0; j < Params.DownsampleFactor; j++)
        {
            // clamp?
            const int2 SourcePos = SourcePosStart + int2(i, j);

            float PrevDepth = ResultDepth;
            ResultDepth = min(ResultDepth, g_depth_buffer[SourcePos].x);
            DepthHistory = min(DepthHistory, g_depth_history[SourcePos].x);

            // override the closest ones
            if (PrevDepth != ResultDepth)
            {
                ResultNormal = g_normals[SourcePos].xyz;
                ResultWorldPos = g_world_pos[SourcePos].xyz;
                ResultVelocity = g_velocity[SourcePos].xyz;
            }
        }
    }

    g_output_depth[dtid] = ResultDepth;
    g_output_normals[dtid] = float4(ResultNormal, 0);
    g_output_world_pos[dtid] = float4(ResultWorldPos, 0);
    g_output_depth_history[dtid] = DepthHistory;
    g_output_velocity[dtid] = float4(ResultVelocity, 0);
}