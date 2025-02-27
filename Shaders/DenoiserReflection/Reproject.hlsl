// https://github.com/GPUOpen-Effects/FidelityFX-SSSR/blob/master/sample/src/Shaders/Reproject.hlsl

#define CBUFFER_SET 0
#define SET 1

#pragma pack_matrix(row_major)

#include "Common.hlsli"

// Inputs
[[vk::binding( 0, SET)]] Texture2D<float>  g_depth_buffer               : register(t0);
[[vk::binding( 1, SET)]] Texture2D<float2> g_roughness                  : register(t1);
[[vk::binding( 2, SET)]] Texture2D<float3> g_normal                     : register(t2);
[[vk::binding( 3, SET)]] Texture2D<float>  g_depth_buffer_history       : register(t3);
[[vk::binding( 4, SET)]] Texture2D<float2> g_roughness_history          : register(t4);
[[vk::binding( 5, SET)]] Texture2D<float3> g_normal_history             : register(t5);

[[vk::binding( 6, SET)]] Texture2D<float4> g_in_radiance                : register(t6);
[[vk::binding( 7, SET)]] Texture2D<float4> g_radiance_history           : register(t7);
[[vk::binding( 8, SET)]] Texture2D<float2> g_motion_vector              : register(t8);

[[vk::binding( 9, SET)]] Texture2D<float3> g_average_radiance_history   : register(t9);
[[vk::binding(10, SET)]] Texture2D<float>  g_variance_history           : register(t10);
[[vk::binding(11, SET)]] Texture2D<float>  g_sample_count_history       : register(t11);
// [[vk::binding(12, SET)]] Texture2D<float2> g_blue_noise_texture         : register(t12);

// Samplers
[[vk::binding(12, SET)]] SamplerState g_linear_sampler                  : register(s12);

// Outputs
[[vk::binding(13, SET)]] RWTexture2D<float4> g_out_reprojected_radiance : register(u0);
[[vk::binding(14, SET)]] RWTexture2D<float4> g_out_average_radiance     : register(u1);
[[vk::binding(15, SET)]] RWTexture2D<float>  g_out_variance             : register(u2);
[[vk::binding(16, SET)]] RWTexture2D<float>  g_out_sample_count         : register(u3);

// [[vk::binding(18, 1)]] Buffer<uint> g_denoiser_tile_list                        : register(t13);

// float       FFX_DNSR_Reflections_GetRandom(int2 pixel_coordinate) { return g_blue_noise_texture.Load(int3(pixel_coordinate.xy % 128, 0)).x; }
float       FFX_DNSR_Reflections_LoadDepth(int2 pixel_coordinate) { return g_depth_buffer.Load(int3(pixel_coordinate, 0)); }
float       FFX_DNSR_Reflections_LoadDepthHistory(int2 pixel_coordinate) { return g_depth_buffer_history.Load(int3(pixel_coordinate, 0)); }
float       FFX_DNSR_Reflections_SampleDepthHistory(float2 uv) { return g_depth_buffer_history.SampleLevel(g_linear_sampler, uv, 0.0f); }
min16float3 FFX_DNSR_Reflections_LoadRadiance(int2 pixel_coordinate) { return (min16float3)g_in_radiance.Load(int3(pixel_coordinate, 0)).xyz; }
min16float3 FFX_DNSR_Reflections_LoadRadianceHistory(int2 pixel_coordinate) { return (min16float3)g_radiance_history.Load(int3(pixel_coordinate, 0)).xyz; }
min16float3 FFX_DNSR_Reflections_SampleRadianceHistory(float2 uv) { return (min16float3)g_radiance_history.SampleLevel(g_linear_sampler, uv, 0.0f).xyz; }
min16float  FFX_DNSR_Reflections_SampleNumSamplesHistory(float2 uv) { return (min16float)g_sample_count_history.SampleLevel(g_linear_sampler, uv, 0.0f).x; }

#if 0
min16float3 FFX_DNSR_Reflections_LoadWorldSpaceNormal(int2 pixel_coordinate) { return normalize(2.0 * (min16float3)g_normal.Load(int3(pixel_coordinate, 0)) - 1.0); }
min16float3 FFX_DNSR_Reflections_LoadWorldSpaceNormalHistory(int2 pixel_coordinate) { return normalize(2.0 * (min16float3)g_normal_history.Load(int3(pixel_coordinate, 0)) - 1.0); }
min16float3 FFX_DNSR_Reflections_SampleWorldSpaceNormalHistory(float2 uv) { return normalize(2.0 * (min16float3)g_normal_history.SampleLevel(g_linear_sampler, uv, 0.0f) - 1.0); }
#else
min16float3 FFX_DNSR_Reflections_LoadWorldSpaceNormal(int2 pixel_coordinate) { return normalize((min16float3)g_normal.Load(int3(pixel_coordinate, 0))); }
min16float3 FFX_DNSR_Reflections_LoadWorldSpaceNormalHistory(int2 pixel_coordinate) { return normalize((min16float3)g_normal_history.Load(int3(pixel_coordinate, 0))); }
min16float3 FFX_DNSR_Reflections_SampleWorldSpaceNormalHistory(float2 uv) { return normalize((min16float3)g_normal_history.SampleLevel(g_linear_sampler, uv, 0.0f)); }
#endif

min16float  FFX_DNSR_Reflections_LoadRoughness(int2 pixel_coordinate) { return (min16float)g_roughness.Load(int3(pixel_coordinate, 0)).r; }
min16float  FFX_DNSR_Reflections_SampleRoughnessHistory(float2 uv) { return (min16float)g_roughness_history.SampleLevel(g_linear_sampler, uv, 0.0f).r; }
min16float  FFX_DNSR_Reflections_LoadRoughnessHistory(int2 pixel_coordinate) { return (min16float)g_roughness_history.Load(int3(pixel_coordinate, 0)).r; }
float2      FFX_DNSR_Reflections_LoadMotionVector(int2 pixel_coordinate) { return g_motion_vector.Load(int3(pixel_coordinate, 0)) * float2(0.5, -0.5); }
min16float3 FFX_DNSR_Reflections_SamplePreviousAverageRadiance(float2 uv) { return (min16float3)g_average_radiance_history.SampleLevel(g_linear_sampler, uv, 0.0f).xyz; }
min16float  FFX_DNSR_Reflections_SampleVarianceHistory(float2 uv) { return (min16float)g_variance_history.SampleLevel(g_linear_sampler, uv, 0.0f).x; }
min16float  FFX_DNSR_Reflections_LoadRayLength(int2 pixel_coordinate) { return (min16float)g_in_radiance.Load(int3(pixel_coordinate, 0)).w; }

void FFX_DNSR_Reflections_StoreRadianceReprojected(int2 pixel_coordinate, min16float3 value) { g_out_reprojected_radiance[pixel_coordinate] = min16float4(value, 1); }
void FFX_DNSR_Reflections_StoreAverageRadiance(int2 pixel_coordinate, min16float3 value) { g_out_average_radiance[pixel_coordinate] = min16float4(value, 1); }
void FFX_DNSR_Reflections_StoreVariance(int2 pixel_coordinate, min16float value) { g_out_variance[pixel_coordinate] = value; }
void FFX_DNSR_Reflections_StoreNumSamples(int2 pixel_coordinate, min16float value) { g_out_sample_count[pixel_coordinate] = value; }

#include "ffx_denoiser_reflections_reproject.h"

[numthreads(8, 8, 1)]
void main(int2 group_thread_id : SV_GroupThreadID, uint group_index : SV_GroupIndex, uint group_id : SV_GroupID, int2 dtid : SV_DispatchThreadID)
{
    // int2  dispatch_thread_id          = int2(packed_coords & 0xffffu, (packed_coords >> 16) & 0xffffu) + group_thread_id;
    int2 dispatch_thread_id = dtid;
    int2  dispatch_group_id           = dispatch_thread_id / 8;
    uint2 remapped_group_thread_id    = FFX_DNSR_Reflections_RemapLane8x8(group_index);
    uint2 remapped_dispatch_thread_id = dispatch_group_id * 8 + remapped_group_thread_id;

    FFX_DNSR_Reflections_Reproject(remapped_dispatch_thread_id, remapped_group_thread_id, g_buffer_dimensions, g_temporal_stability_factor, 32);
}