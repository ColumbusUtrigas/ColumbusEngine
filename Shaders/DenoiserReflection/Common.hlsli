// https://github.com/GPUOpen-Effects/FidelityFX-SSSR/blob/master/sample/src/Shaders/Common.hlsl

static const float g_roughness_sigma_min = 0.01f;
static const float g_roughness_sigma_max = 0.02f;
static const float g_depth_sigma = 0.02f;

[[vk::binding(0, CBUFFER_SET)]] cbuffer Constants : register(b0) {
    float4x4 g_inv_view_proj;
    float4x4 g_proj;
    float4x4 g_inv_proj;
    float4x4 g_view;
    float4x4 g_inv_view;
    float4x4 g_prev_view_proj;
    uint2 g_buffer_dimensions;
    float2 g_inv_buffer_dimensions;
    float g_temporal_stability_factor;
    float g_depth_buffer_thickness;
    float g_roughness_threshold;
    float g_temporal_variance_threshold;
    uint g_frame_index;
    uint g_max_traversal_intersections;
    uint g_min_traversal_occupancy;
    uint g_most_detailed_mip;
    uint g_samples_per_quad;
    uint g_temporal_variance_guided_tracing_enabled;
};

// Transforms origin to uv space
// Mat must be able to transform origin from its current space into clip space.
float3 ProjectPosition(float3 origin, float4x4 mat) {
    float4 projected = mul(mat, float4(origin, 1));
    projected.xyz /= projected.w;
    projected.xy = 0.5 * projected.xy + 0.5;
    projected.y = (1 - projected.y);
    return projected.xyz;
}

// Origin and direction must be in the same space and mat must be able to transform from that space into clip space.
float3 ProjectDirection(float3 origin, float3 direction, float3 screen_space_origin, float4x4 mat) {
    float3 offsetted = ProjectPosition(origin + direction, mat);
    return offsetted - screen_space_origin;
}

// Mat must be able to transform origin from texture space to a linear space.
float3 InvProjectPosition(float3 coord, float4x4 mat) {
    coord.y = (1 - coord.y);
    coord.xy = 2 * coord.xy - 1;
    float4 projected = mul(mat, float4(coord, 1));
    projected.xyz /= projected.w;
    return projected.xyz;
}

//=== FFX_DNSR_Reflections_ override functions ===

bool FFX_DNSR_Reflections_IsGlossyReflection(float roughness) {
    return roughness <= g_roughness_threshold;
}

bool FFX_DNSR_Reflections_IsMirrorReflection(float roughness) {
    return roughness <= 0.01;
}

float3 FFX_DNSR_Reflections_ScreenSpaceToViewSpace(float3 screen_uv_coord) {
    return InvProjectPosition(screen_uv_coord, g_inv_proj);
}

float3 FFX_DNSR_Reflections_ViewSpaceToWorldSpace(float4 view_space_coord) {
    return mul(g_inv_view, view_space_coord).xyz;
}

float3 FFX_DNSR_Reflections_WorldSpaceToScreenSpacePrevious(float3 world_space_pos) {
    return ProjectPosition(world_space_pos, g_prev_view_proj);
}

float FFX_DNSR_Reflections_GetLinearDepth(float2 uv, float depth) {
    const float3 view_space_pos = InvProjectPosition(float3(uv, depth), g_inv_proj);
    return abs(view_space_pos.z);
}

uint FFX_DNSR_Reflections_RoundedDivide(uint value, uint divisor) {
    return (value + divisor - 1) / divisor;
}
