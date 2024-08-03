// https://github.com/GPUOpen-Effects/FidelityFX-SSSR/blob/master/sample/src/Shaders/ResolveTemporal.hlsl

#define CBUFFER_SET 0
#define SET 1

#pragma pack_matrix(row_major)

#include "Common.hlsli"

// Inputs
[[vk::binding( 0, 1)]] Texture2D<float> g_roughness                             : register(t0);
[[vk::binding( 1, 1)]] Texture2D<float3> g_average_radiance                     : register(t1);
[[vk::binding( 2, 1)]] Texture2D<float4> g_in_radiance                          : register(t2);
[[vk::binding( 3, 1)]] Texture2D<float4> g_in_reprojected_radiance              : register(t3);
[[vk::binding( 4, 1)]] Texture2D<float> g_in_variance                           : register(t4);
[[vk::binding( 5, 1)]] Texture2D<float> g_in_sample_count                       : register(t5);

// Samplers
[[vk::binding( 6, 1)]] SamplerState g_linear_sampler                            : register(s0);

// Outputs
[[vk::binding( 7, 1)]] RWTexture2D<float4> g_out_radiance                       : register(u0);
[[vk::binding( 8, 1)]] RWTexture2D<float> g_out_variance                        : register(u1);
[[vk::binding( 9, 1)]] RWTexture2D<float> g_out_sample_count                    : register(u2);

// [[vk::binding(10, 1)]] Buffer<uint> g_denoiser_tile_list                        : register(t6);

min16float3 FFX_DNSR_Reflections_SampleAverageRadiance(float2 uv) { return (min16float3)g_average_radiance.SampleLevel(g_linear_sampler, uv, 0.0f).xyz; }
min16float3 FFX_DNSR_Reflections_LoadRadiance(int2 pixel_coordinate) { return (min16float3)g_in_radiance.Load(int3(pixel_coordinate, 0)).xyz; }
min16float3 FFX_DNSR_Reflections_LoadRadianceReprojected(int2 pixel_coordinate) { return (min16float3)g_in_reprojected_radiance.Load(int3(pixel_coordinate, 0)).xyz; }
min16float FFX_DNSR_Reflections_LoadRoughness(int2 pixel_coordinate) { return (min16float)g_roughness.Load(int3(pixel_coordinate, 0)); }
min16float FFX_DNSR_Reflections_LoadVariance(int2 pixel_coordinate) { return (min16float)g_in_variance.Load(int3(pixel_coordinate, 0)).x; }
min16float FFX_DNSR_Reflections_LoadNumSamples(int2 pixel_coordinate) { return (min16float)g_in_sample_count.Load(int3(pixel_coordinate, 0)).x; }
void FFX_DNSR_Reflections_StoreTemporalAccumulation(int2 pixel_coordinate, min16float3 radiance, min16float variance) {
    g_out_radiance[pixel_coordinate] = radiance.xyzz;
    g_out_variance[pixel_coordinate] = variance.x;
}

#include "ffx_denoiser_reflections_resolve_temporal.h"

[numthreads(8, 8, 1)]
void main(int2 group_thread_id      : SV_GroupThreadID,
                uint group_index    : SV_GroupIndex,
                uint    group_id    : SV_GroupID,
                int2 dtid           : SV_DispatchThreadID) {
    // uint  packed_coords               = g_denoiser_tile_list[group_id];
    // int2  dispatch_thread_id          = int2(packed_coords & 0xffffu, (packed_coords >> 16) & 0xffffu) + group_thread_id;
    int2  dispatch_thread_id = dtid;
    int2  dispatch_group_id           = dispatch_thread_id / 8;
    uint2 remapped_group_thread_id    = FFX_DNSR_Reflections_RemapLane8x8(group_index);
    uint2 remapped_dispatch_thread_id = dispatch_group_id * 8 + remapped_group_thread_id;

    FFX_DNSR_Reflections_ResolveTemporal(remapped_dispatch_thread_id, remapped_group_thread_id, g_buffer_dimensions, g_inv_buffer_dimensions, g_temporal_stability_factor);
}
