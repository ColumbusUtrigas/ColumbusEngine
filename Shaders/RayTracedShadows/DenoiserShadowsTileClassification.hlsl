/**********************************************************************
Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/

#ifndef FFX_DNSR_SHADOWS_TILECLASSIFICATION_HLSL
#define FFX_DNSR_SHADOWS_TILECLASSIFICATION_HLSL

#pragma pack_matrix(row_major)

#include "DenoiserShadowsUtil.hlsli"

// begin user code

[[vk::push_constant]]
struct _Params {
	float4x4 InvViewProjectionMatrix;
	float4x4 InvProjectionMatrix;
	float4x4 ViewToPrevViewMatrix;
	float3 CameraPosition;
	int2 BufferDimensions;
	int2 PackedBufferDimensions;
	int FirstFrame;
} Params;

RWTexture2D<uint> PackedTilesBuffer : register(u0);

Texture2D<float3> Normals : register(t1);
Texture2D<float> Depth : register(t2);
Texture2D<float> DepthHistory : register(t3);
Texture2D<float2> Velocity : register(t4);

[[vk::image_format("rg16f")]]      RWTexture2D<float2> ReprojectionResult : register(u5);
[[vk::image_format("r11g11b10f")]] RWTexture2D<float3> Moments : register(u6);
[[vk::image_format("r11g11b10f")]] RWTexture2D<float3> MomentsHistory : register(u7);
[[vk::image_format("r8ui")]]       RWTexture2D<uint> Metadata : register(u8);
Texture2D<float2> History : register(t9);
SamplerState HistorySampler : register(s10);

uint2 FFX_DNSR_Shadows_GetBufferDimensions()
{
	return Params.BufferDimensions;
}

float2 FFX_DNSR_Shadows_GetInvBufferDimensions()
{
	return 1.0 / float2(Params.BufferDimensions);
}

float4x4 FFX_DNSR_Shadows_GetViewProjectionInverse()
{
	return Params.InvViewProjectionMatrix;
}

float4x4 FFX_DNSR_Shadows_GetProjectionInverse()
{
	return Params.InvProjectionMatrix;
}

float4x4 FFX_DNSR_Shadows_GetViewToPrevView()
{
	return Params.ViewToPrevViewMatrix;
}

float3 FFX_DNSR_Shadows_GetEye()
{
	return Params.CameraPosition;
}

float FFX_DNSR_Shadows_ReadHistory(float2 history_uv)
{
	// History is filter pass-0 output (mean, variance) in rg16f; .x is the mean.
	return History.SampleLevel(HistorySampler, history_uv, 0).x;
}

float FFX_DNSR_Shadows_ReadDepth(uint2 did)
{
	did = min(did, FFX_DNSR_Shadows_GetBufferDimensions() - 1);
	return Depth[did];
}

float FFX_DNSR_Shadows_ReadPreviousDepth(int2 idx)
{
	idx = clamp(idx, int2(0, 0), int2(FFX_DNSR_Shadows_GetBufferDimensions()) - 1);
	return DepthHistory[idx];
}

float3 FFX_DNSR_Shadows_ReadNormals(uint2 did)
{
	did = min(did, FFX_DNSR_Shadows_GetBufferDimensions() - 1);
	return Normals[did];
}

float2 FFX_DNSR_Shadows_ReadVelocity(uint2 did)
{
	did = min(did, FFX_DNSR_Shadows_GetBufferDimensions() - 1);
	return Velocity[did].xy;
}

uint FFX_DNSR_Shadows_ReadRaytracedShadowMask(uint linear_tile_index)
{
	uint width = Params.PackedBufferDimensions.x;
	uint2 coords = uint2(linear_tile_index % width, linear_tile_index / width);
	return PackedTilesBuffer[coords];
}

void FFX_DNSR_Shadows_WriteMetadata(uint2 idx, uint mask)
{
	Metadata[idx] = mask;
}

void FFX_DNSR_Shadows_WriteReprojectionResults(uint2 did, float2 value)
{
	if (any(did >= FFX_DNSR_Shadows_GetBufferDimensions()))
		return;

	ReprojectionResult[did] = value;
}

void FFX_DNSR_Shadows_WriteMoments(uint2 did, float3 value)
{
	if (any(did >= FFX_DNSR_Shadows_GetBufferDimensions()))
		return;

	Moments[did] = value;
}

float3 FFX_DNSR_Shadows_ReadPreviousMomentsBuffer(int2 history_pos)
{
	history_pos = clamp(history_pos, int2(0, 0), int2(FFX_DNSR_Shadows_GetBufferDimensions()) - 1);
	return MomentsHistory[history_pos];
}

bool FFX_DNSR_Shadows_IsShadowReciever(uint2 did)
{
	if (any(did >= FFX_DNSR_Shadows_GetBufferDimensions()))
		return false;

	float depth = FFX_DNSR_Shadows_ReadDepth(did);
	return (depth > 0.0f) && (depth < 1.0f);
}

int FFX_DNSR_Shadows_IsFirstFrame()
{
	return Params.FirstFrame == 1;
}

// end user code

groupshared int g_FFX_DNSR_Shadows_false_count;
bool FFX_DNSR_Shadows_ThreadGroupAllTrue(bool val)
{
	const uint lane_count_in_thread_group = 64;
	if (WaveGetLaneCount() == lane_count_in_thread_group)
	{
		return WaveActiveAllTrue(val);
	}
	else
	{
		GroupMemoryBarrierWithGroupSync();
		g_FFX_DNSR_Shadows_false_count = 0;
		GroupMemoryBarrierWithGroupSync();
		if (!val) g_FFX_DNSR_Shadows_false_count = 1;
		GroupMemoryBarrierWithGroupSync();
		return g_FFX_DNSR_Shadows_false_count == 0;
	}
}

void FFX_DNSR_Shadows_SearchSpatialRegion(uint2 gid, out bool all_in_light, out bool all_in_shadow)
{
	// The spatial passes can reach a total region of 1+2+4 = 7x7 around each block.
	// The masks are 8x4, so we need a larger vertical stride

	// Visualization - each x represents a 4x4 block, xx is one entire 8x4 mask as read from the raytracer result
	// Same for yy, these are the ones we are working on right now

	// xx xx xx
	// xx xx xx
	// xx yy xx <-- yy here is the base_tile below
	// xx yy xx
	// xx xx xx
	// xx xx xx

	// All of this should result in scalar ops
	uint2 base_tile = FFX_DNSR_Shadows_GetTileIndexFromPixelPosition(gid * int2(8, 8));

	// Load the entire region of masks in a scalar fashion
	uint combined_or_mask = 0;
	uint combined_and_mask = 0xFFFFFFFF;
	for (int j = -2; j <= 3; ++j)
	{
		for (int i = -1; i <= 1; ++i)
		{
			int2 tile_index = base_tile + int2(i, j);
			tile_index = clamp(tile_index, 0, int2(FFX_DNSR_Shadows_RoundedDivide(FFX_DNSR_Shadows_GetBufferDimensions().x, 8), FFX_DNSR_Shadows_RoundedDivide(FFX_DNSR_Shadows_GetBufferDimensions().y, 4)) - 1);
			const uint linear_tile_index = FFX_DNSR_Shadows_LinearTileIndex(tile_index, FFX_DNSR_Shadows_GetBufferDimensions().x);
			const uint shadow_mask = FFX_DNSR_Shadows_ReadRaytracedShadowMask(linear_tile_index);

			combined_or_mask = combined_or_mask | shadow_mask;
			combined_and_mask = combined_and_mask & shadow_mask;
		}
	}

	all_in_light = combined_and_mask == 0xFFFFFFFFu;
	all_in_shadow = combined_or_mask == 0u;
}

float FFX_DNSR_Shadows_GetLinearDepth(uint2 did, float depth)
{
	const float2 uv = (did + 0.5f) * FFX_DNSR_Shadows_GetInvBufferDimensions();
	const float2 ndc = 2.0f * float2(uv.x, 1.0f - uv.y) - 1.0f;
   
	float4 projected = mul(FFX_DNSR_Shadows_GetProjectionInverse(), float4(ndc, depth, 1));
	return abs(projected.z / projected.w);
}

// VELOCITY_CONVENTION:
//   0 = ENGINE CANONICAL: uv - vel * 0.5, NO Y-flip. vel = current_NDC - previous_NDC.
//       Matches TAA, RTGI Temporal, and Reflections denoiser exactly.
//   1 = FFX style: uv + vel (no scaling), vel = previous_UV - current_UV
//   2 = NDC prev-cur: uv + vel * 0.5, vel = previous_NDC - current_NDC
//   3 = FORCE ZERO (diagnostic: if disocclusion disappears, velocity is the root cause)
#define VELOCITY_CONVENTION 0

// BYPASS_DISOCCLUSION: set to 1 to force is_disoccluded = false everywhere.
// If variance drops to sane values with this on, disocclusion is confirmed as the root cause.
#define BYPASS_DISOCCLUSION 0

// BYPASS_COUNTER_DAMPER: set to 1 to skip the sample counter damper.
// The FFX damper interprets ordinary 1-spp penumbra noise as a discontinuity and caps
// history length (~5-6 frames) at penumbrae, which collapses history_weight and makes the
// blend favor the raw noisy sample -> jitter. Keep OFF until reworked to fire only on
// genuine discontinuities. (FFX path: SIMPLE_TEMPORAL 0.)
#define BYPASS_COUNTER_DAMPER 0

// SIMPLE_TEMPORAL: clean no-reprojection/no-rejection baseline (diagnostic only).
// Outputs the Welford running mean (moments.x) directly via NEAREST-NEIGHBOR moment reprojection,
// which floats/snaps during motion. Keep OFF for the proper FFX path: bilinearly-sampled
// History.Shadow + spatial clamp (motion-robust).
#define SIMPLE_TEMPORAL 0

// On-screen debug readouts (set ONE to 1, keep DEBUG_VIZ 0). They override the final shadow
// output directly so you can see them on screen without RenderDoc.
#define DEBUG_SHOW_HISTORY        0  // shows shadow_previous (the value read from History.Shadow)
#define DEBUG_SHOW_HISTORY_WEIGHT 0  // shows how much the blend favors history (bright=history)
#define DEBUG_SHOW_FIRSTFRAME     0  // shows IsFirstFrame() (bright every frame = stuck true)

// Disocclusion depth-rejection tuning (SIGMA-style slope-scaled tolerance).
// tolerance = BASE / max(0.05, NoV), clamped to MAX. Grazing surfaces (low NoV, e.g. floors)
// get proportionally more slack, which is what SIGMA does (GetDisocclusionThreshold:
// frustumSize * saturate(threshold / max(0.05, NoV))). This replaces FFX's pow(zAlign,8)
// which only loosened at extreme grazing angles and caused floor streaks.
#define DISOCCLUSION_TOLERANCE_BASE 0.02f // head-on (NoV=1) relative depth tolerance
#define DISOCCLUSION_TOLERANCE_MAX  0.25f // cap for very grazing angles

struct FFX_DNSR_Shadows_DisocclusionResult
{
	bool is_disoccluded;
	float depth_difference;
	float depth_tolerance;
	float linear_depth;
	float previous_linear_depth;
	float z_alignment;
	float clip_space_z;
	float2 reprojected_ndc_xy;
	float4 clip_space_before_divide;
};

FFX_DNSR_Shadows_DisocclusionResult FFX_DNSR_Shadows_IsDisoccludedEx(uint2 did, float depth, float2 velocity)
{
	FFX_DNSR_Shadows_DisocclusionResult result;
	result.is_disoccluded = true;
	result.depth_difference = 999.0f;
	result.depth_tolerance = 0.0f;
	result.linear_depth = 0.0f;
	result.previous_linear_depth = 0.0f;
	result.z_alignment = 0.0f;
	result.clip_space_z = 0.0f;
	result.reprojected_ndc_xy = float2(0, 0);
	result.clip_space_before_divide = float4(0, 0, 0, 0);

	const int2 dims = FFX_DNSR_Shadows_GetBufferDimensions();
	const float2 texel_size = FFX_DNSR_Shadows_GetInvBufferDimensions();
	const float2 uv = (did + 0.5f) * texel_size;
	const float2 ndc = (2.0f * uv - 1.0f) * float2(1.0f, -1.0f);
#if VELOCITY_CONVENTION == 0 || VELOCITY_CONVENTION == 2
	const float2 previous_uv = uv - velocity;
#elif VELOCITY_CONVENTION == 1
	const float2 previous_uv = uv + velocity;
#elif VELOCITY_CONVENTION == 3
	const float2 previous_uv = uv;
#endif

	if (all(previous_uv > 0.0) && all(previous_uv < 1.0))
	{
		float3 normal = normalize(FFX_DNSR_Shadows_ReadNormals(did));

		// Reconstruct current view-space position (camera-relative, small coords -> precise far from origin).
		float4 view_h = mul(FFX_DNSR_Shadows_GetProjectionInverse(), float4(ndc, depth, 1.0f));
		float3 view_pos = view_h.xyz / view_h.w;

		// Transform into PREVIOUS view space via the camera-relative delta transform.
		// (Mathematically equal to PrevVP*InvCurVP reprojection, but kept camera-relative so it
		//  doesn't lose precision when the camera is far from the world origin.)
		float3 prev_view_pos = mul(FFX_DNSR_Shadows_GetViewToPrevView(), float4(view_pos, 1.0f)).xyz;

		// Reprojected previous-frame linear depth (view looks down -Z, so use magnitude).
		float linear_depth = abs(prev_view_pos.z);
		result.linear_depth = linear_depth;
		result.clip_space_z = linear_depth;            // debug reuse
		result.clip_space_before_divide = float4(prev_view_pos, 1.0f); // debug reuse
		result.reprojected_ndc_xy = previous_uv;       // debug reuse

		// View direction / NoV for the slope-scaled tolerance (world-space; only affects tolerance,
		// so the precision here is non-critical).
		const float4 homogeneous = mul(FFX_DNSR_Shadows_GetViewProjectionInverse(), float4(ndc, depth, 1.0f));
		const float3 world_position = homogeneous.xyz / homogeneous.w;
		const float3 view_direction = normalize(FFX_DNSR_Shadows_GetEye().xyz - world_position);
		float NoV = saturate(dot(view_direction, normal));
		result.z_alignment = NoV; // debug field reused to show NoV

		int2 idx = previous_uv * dims;
		const float previous_depth = FFX_DNSR_Shadows_GetLinearDepth(idx, FFX_DNSR_Shadows_ReadPreviousDepth(idx));
		result.previous_linear_depth = previous_depth;
		const float depth_difference = abs(previous_depth - linear_depth) / linear_depth;
		result.depth_difference = depth_difference;

		// SIGMA-style slope-scaled tolerance (Common.hlsli GetDisocclusionThreshold).
		const float depth_tolerance = min(DISOCCLUSION_TOLERANCE_BASE / max(0.05f, NoV), DISOCCLUSION_TOLERANCE_MAX);
		result.depth_tolerance = depth_tolerance;
		result.is_disoccluded = depth_difference >= depth_tolerance;
	}

#if BYPASS_DISOCCLUSION
	result.is_disoccluded = false;
#endif

	return result;
}

float2 FFX_DNSR_Shadows_GetClosestVelocity(int2 did, float depth)
{
	float2 closest_velocity = FFX_DNSR_Shadows_ReadVelocity(did);
	float closest_depth = depth;

	float new_depth = QuadReadAcrossX(closest_depth);
	float2 new_velocity = QuadReadAcrossX(closest_velocity);
#ifdef INVERTED_DEPTH_RANGE
	if (new_depth > closest_depth)
#else
	if (new_depth < closest_depth)
#endif
	{
		closest_depth = new_depth;
		closest_velocity = new_velocity;
	}

	new_depth = QuadReadAcrossY(closest_depth);
	new_velocity = QuadReadAcrossY(closest_velocity);
#ifdef INVERTED_DEPTH_RANGE
	if (new_depth > closest_depth)
#else
	if (new_depth < closest_depth)
#endif
	{
		closest_depth = new_depth;
		closest_velocity = new_velocity;
	}
	
#if VELOCITY_CONVENTION == 0
	// Engine canonical: vel = current_NDC - previous_NDC. UV delta = vel * 0.5, no Y-flip.
	// Reprojection uses uv - vel (see history_uv / previous_uv below). Matches TAA/RTGI/Reflections.
	return closest_velocity * 0.5f;
#elif VELOCITY_CONVENTION == 1
	// FFX: vel = previous_UV - current_UV, no scaling
	return closest_velocity;
#elif VELOCITY_CONVENTION == 2
	// vel = previous_NDC - current_NDC, uv_delta = +vel * 0.5
	return float2(-closest_velocity.x, closest_velocity.y) * 0.5f;
#elif VELOCITY_CONVENTION == 3
	return 0.0f;
#endif
}

#define KERNEL_RADIUS 8
float FFX_DNSR_Shadows_KernelWeight(float i)
{
#define KERNEL_WEIGHT(i) (exp(-3.0 * float(i * i) / ((KERNEL_RADIUS + 1.0) * (KERNEL_RADIUS + 1.0))))

	// Statically initialize kernel_weights_sum
	float kernel_weights_sum = 0;
	kernel_weights_sum += KERNEL_WEIGHT(0);
	for (int c = 1; c <= KERNEL_RADIUS; ++c)
	{
		kernel_weights_sum += 2 * KERNEL_WEIGHT(c); // Add other half of the kernel to the sum
	}
	float inv_kernel_weights_sum = rcp(kernel_weights_sum);

	// The only runtime code in this function
	return KERNEL_WEIGHT(i) * inv_kernel_weights_sum;
}

void FFX_DNSR_Shadows_AccumulateMoments(float value, float weight, inout float moments)
{
	// We get value from the horizontal neighborhood calculations. Thus, it's both mean and variance due to using one sample per pixel
	moments += value * weight;
}

// The horizontal part of a 17x17 local neighborhood kernel
float FFX_DNSR_Shadows_HorizontalNeighborhood(int2 did)
{
   const int2 base_did = did;

	// Prevent vertical out of bounds access (FFX only guards Y; X is handled by the
	// left/center/right tile zeroing below via is_first/is_last_tile_in_row).
	if ((base_did.y < 0) || (base_did.y >= FFX_DNSR_Shadows_GetBufferDimensions().y)) return 0;


	const uint2 tile_index = FFX_DNSR_Shadows_GetTileIndexFromPixelPosition(base_did);
	const uint linear_tile_index = FFX_DNSR_Shadows_LinearTileIndex(tile_index, FFX_DNSR_Shadows_GetBufferDimensions().x);

	const int left_tile_index = linear_tile_index - 1;
	const int center_tile_index = linear_tile_index;
	const int right_tile_index = linear_tile_index + 1;

	bool is_first_tile_in_row = tile_index.x == 0;
	bool is_last_tile_in_row = tile_index.x == (FFX_DNSR_Shadows_RoundedDivide(FFX_DNSR_Shadows_GetBufferDimensions().x, 8) - 1);

	uint left_tile = 0;
	if (!is_first_tile_in_row) left_tile = FFX_DNSR_Shadows_ReadRaytracedShadowMask(left_tile_index);
	uint center_tile = FFX_DNSR_Shadows_ReadRaytracedShadowMask(center_tile_index);
	uint right_tile = 0;
	if (!is_last_tile_in_row) right_tile = FFX_DNSR_Shadows_ReadRaytracedShadowMask(right_tile_index);

	// Construct a single uint with the lowest 17bits containing the horizontal part of the local neighborhood.

	// First extract the 8 bits of our row in each of the neighboring tiles
	const uint row_base_index = (did.y % 4) * 8;
	const uint left = (left_tile >> row_base_index) & 0xFF;
	const uint center = (center_tile >> row_base_index) & 0xFF;
	const uint right = (right_tile >> row_base_index) & 0xFF;

	// Combine them into a single mask containting [left, center, right] from least significant to most significant bit
	uint neighborhood = left | (center << 8) | (right << 16);

	// Make sure our pixel is at bit position 9 to get the highest contribution from the filter kernel
	const uint bit_index_in_row = (did.x % 8);
	neighborhood = neighborhood >> bit_index_in_row; // Shift out bits to the right, so the center bit ends up at bit 9.

	float moment = 0.0; // For one sample per pixel this is both, mean and variance

	// First 8 bits up to the center pixel
	uint mask;
	int i;
	for (i = 0; i < 8; ++i)
	{
		mask = 1u << i;
		moment += (mask & neighborhood) ? FFX_DNSR_Shadows_KernelWeight(8 - i) : 0;
	}

	// Center pixel
	mask = 1u << 8;
	moment += (mask & neighborhood) ? FFX_DNSR_Shadows_KernelWeight(0) : 0;

	// Last 8 bits
	for (i = 1; i <= 8; ++i)
	{
		mask = 1u << (8 + i);
		moment += (mask & neighborhood) ? FFX_DNSR_Shadows_KernelWeight(i) : 0;
	}

	return moment;
}

groupshared float g_FFX_DNSR_Shadows_neighborhood[8][24];

float FFX_DNSR_Shadows_ComputeLocalNeighborhood(int2 did, int2 gtid)
{
	float local_neighborhood = 0;

	float upper = FFX_DNSR_Shadows_HorizontalNeighborhood(int2(did.x, did.y - 8));
	float center = FFX_DNSR_Shadows_HorizontalNeighborhood(int2(did.x, did.y));
	float lower = FFX_DNSR_Shadows_HorizontalNeighborhood(int2(did.x, did.y + 8));

	g_FFX_DNSR_Shadows_neighborhood[gtid.x][gtid.y] = upper;
	g_FFX_DNSR_Shadows_neighborhood[gtid.x][gtid.y + 8] = center;
	g_FFX_DNSR_Shadows_neighborhood[gtid.x][gtid.y + 16] = lower;

	GroupMemoryBarrierWithGroupSync();

	// First combine the own values.
	// KERNEL_RADIUS pixels up is own upper and KERNEL_RADIUS pixels down is own lower value
	FFX_DNSR_Shadows_AccumulateMoments(center, FFX_DNSR_Shadows_KernelWeight(0), local_neighborhood);
	FFX_DNSR_Shadows_AccumulateMoments(upper, FFX_DNSR_Shadows_KernelWeight(KERNEL_RADIUS), local_neighborhood);
	FFX_DNSR_Shadows_AccumulateMoments(lower, FFX_DNSR_Shadows_KernelWeight(KERNEL_RADIUS), local_neighborhood);

	// Then read the neighboring values.
	for (int i = 1; i < KERNEL_RADIUS; ++i)
	{
		float upper_value = g_FFX_DNSR_Shadows_neighborhood[gtid.x][8 + gtid.y - i];
		float lower_value = g_FFX_DNSR_Shadows_neighborhood[gtid.x][8 + gtid.y + i];
		float weight = FFX_DNSR_Shadows_KernelWeight(i);
		FFX_DNSR_Shadows_AccumulateMoments(upper_value, weight, local_neighborhood);
		FFX_DNSR_Shadows_AccumulateMoments(lower_value, weight, local_neighborhood);
	}

	return local_neighborhood;
}

void FFX_DNSR_Shadows_WriteTileMetaData(uint2 gid, uint2 gtid, bool is_cleared, bool all_in_light)
{
	if (all(gtid == 0))
	{
		uint light_mask = all_in_light ? TILE_META_DATA_LIGHT_MASK : 0;
		uint clear_mask = is_cleared ? TILE_META_DATA_CLEAR_MASK : 0;
		uint mask = light_mask | clear_mask;
		// FFX_DNSR_Shadows_WriteMetadata(gid.y * FFX_DNSR_Shadows_RoundedDivide(FFX_DNSR_Shadows_GetBufferDimensions().x, 8) + gid.x, mask);
		FFX_DNSR_Shadows_WriteMetadata(gid, mask);
	}
}

void FFX_DNSR_Shadows_ClearTargets(uint2 did, uint2 gtid, uint2 gid, float shadow_value, bool is_shadow_receiver, bool all_in_light)
{
	FFX_DNSR_Shadows_WriteTileMetaData(gid, gtid, true, all_in_light);
	FFX_DNSR_Shadows_WriteReprojectionResults(did, float2(shadow_value, 0)); // mean, variance

	float temporal_sample_count = is_shadow_receiver ? 1 : 0;
	FFX_DNSR_Shadows_WriteMoments(did, float3(shadow_value, 0, temporal_sample_count));// mean, variance, temporal sample count
}



#define DEBUG_VIZ 0

#if DEBUG_VIZ != 0
groupshared float2 g_dbg_velocity[8][8];
groupshared bool   g_dbg_disoccluded[8][8];
groupshared float  g_dbg_old_s[8][8];
groupshared float  g_dbg_sample_count[8][8];
groupshared float  g_dbg_temporal_var[8][8];
groupshared float  g_dbg_spatial_var[8][8];
groupshared float  g_dbg_shadow_previous[8][8];
groupshared float  g_dbg_history_uv_valid[8][8];
#endif

void FFX_DNSR_Shadows_TileClassification(uint group_index, uint2 gid)
{
	uint2 gtid = FFX_DNSR_Shadows_RemapLane8x8(group_index);
	uint2 did = gid * 8 + gtid;

	bool is_shadow_receiver = FFX_DNSR_Shadows_IsShadowReciever(did);

	bool skip_sky = FFX_DNSR_Shadows_ThreadGroupAllTrue(!is_shadow_receiver);
	if (skip_sky)
	{
		FFX_DNSR_Shadows_ClearTargets(did, gtid, gid, 0, is_shadow_receiver, false);
		return;
	}

	bool all_in_light = false;
	bool all_in_shadow = false;
	FFX_DNSR_Shadows_SearchSpatialRegion(gid, all_in_light, all_in_shadow);
	float shadow_value = all_in_light ? 1 : 0;

	bool can_skip = all_in_light || all_in_shadow;
	bool skip_tile = FFX_DNSR_Shadows_ThreadGroupAllTrue(can_skip);
	if (skip_tile)
	{
		FFX_DNSR_Shadows_ClearTargets(did, gtid, gid, shadow_value, is_shadow_receiver, all_in_light);
		return;
	}

	FFX_DNSR_Shadows_WriteTileMetaData(gid, gtid, false, false);

	float depth = FFX_DNSR_Shadows_ReadDepth(did);
	const float2 velocity = FFX_DNSR_Shadows_GetClosestVelocity(did.xy, depth);
	const float local_neighborhood = FFX_DNSR_Shadows_ComputeLocalNeighborhood(did, gtid);

	const float2 texel_size = FFX_DNSR_Shadows_GetInvBufferDimensions();
	const float2 uv = (did.xy + 0.5f) * texel_size;
#if VELOCITY_CONVENTION == 0 || VELOCITY_CONVENTION == 2
	const float2 history_uv = uv - velocity;
#elif VELOCITY_CONVENTION == 1
	const float2 history_uv = uv + velocity;
#elif VELOCITY_CONVENTION == 3
	const float2 history_uv = uv;
#endif
	const int2 history_pos = history_uv * FFX_DNSR_Shadows_GetBufferDimensions();

	const uint2 tile_index = FFX_DNSR_Shadows_GetTileIndexFromPixelPosition(did);
	const uint linear_tile_index = FFX_DNSR_Shadows_LinearTileIndex(tile_index, FFX_DNSR_Shadows_GetBufferDimensions().x);

	const uint shadow_tile = FFX_DNSR_Shadows_ReadRaytracedShadowMask(linear_tile_index);

	float3 moments_current = 0;
	float variance = 0;
	float shadow_clamped = 0;
	float temporal_variance_pre_boost = 0;
	float spatial_variance_dbg = 0;
	bool is_disoccluded_dbg = false;
	float old_s_dbg = 0;
	float sample_count_dbg = 0;
	float shadow_previous_dbg = 0;
	float history_uv_valid_dbg = 0;
	float depth_difference_dbg = 0;
	float depth_tolerance_dbg = 0;
	float linear_depth_dbg = 0;
	float prev_linear_depth_dbg = 0;
	float z_alignment_dbg = 0;
	float clip_space_z_dbg = 0;
	float2 reprojected_ndc_xy_dbg = 0;
	float4 clip_space_raw_dbg = 0;
	float temporal_discontinuity_dbg = 0;
	float counter_damper_dbg = 0;

	if (is_shadow_receiver)
	{
		bool hit_light = shadow_tile & FFX_DNSR_Shadows_GetBitMaskFromPixelPosition(did);
		const float shadow_current = hit_light ? 1.0 : 0.0;

		{
			FFX_DNSR_Shadows_DisocclusionResult disoc = FFX_DNSR_Shadows_IsDisoccludedEx(did, depth, velocity);
			is_disoccluded_dbg = disoc.is_disoccluded;
			depth_difference_dbg = disoc.depth_difference;
			depth_tolerance_dbg = disoc.depth_tolerance;
			linear_depth_dbg = disoc.linear_depth;
			prev_linear_depth_dbg = disoc.previous_linear_depth;
			z_alignment_dbg = disoc.z_alignment;
			clip_space_z_dbg = disoc.clip_space_z;
			reprojected_ndc_xy_dbg = disoc.reprojected_ndc_xy;
			clip_space_raw_dbg = disoc.clip_space_before_divide;
			const float3 previous_moments = disoc.is_disoccluded ? float3(0.0f, 0.0f, 0.0f)
				: FFX_DNSR_Shadows_ReadPreviousMomentsBuffer(history_pos);

			const float old_m = previous_moments.x;
			const float old_s = previous_moments.y;
			old_s_dbg = old_s;
#if SIMPLE_TEMPORAL && 0
			// Cap the sample count so the running average becomes an exponential moving
			// average (min blend = 1/MAX). Prevents the mean from freezing and stays
			// responsive. 32 -> ~3% minimum weight on the newest sample.
			const float MAX_SAMPLES = 32.0f;
			const float sample_count = min(previous_moments.z + 1.0f, MAX_SAMPLES);
#else
			const float sample_count = previous_moments.z + 1.0f;
#endif
			sample_count_dbg = sample_count;
			const float new_m = old_m + (shadow_current - old_m) / sample_count;
			const float new_s = old_s + (shadow_current - old_m) * (shadow_current - new_m);

			temporal_variance_pre_boost = (sample_count > 1.0f ? new_s / (sample_count - 1.0f) : 1.0f);
			variance = temporal_variance_pre_boost;
			moments_current = float3(new_m, new_s, sample_count);
		}

		{
			float mean = local_neighborhood;
			float spatial_variance = local_neighborhood;

			spatial_variance = max(spatial_variance - mean * mean, 0.0f);
			spatial_variance_dbg = spatial_variance;

			const float std_deviation = sqrt(spatial_variance);
			const float nmin = mean - 0.5f * std_deviation;
			const float nmax = mean + 0.5f * std_deviation;

			float shadow_previous = shadow_current;
			history_uv_valid_dbg = (all(history_uv > 0.0) && all(history_uv < 1.0)) ? 1.0 : 0.0;
			if (FFX_DNSR_Shadows_IsFirstFrame() == 0)
			{
				shadow_previous = FFX_DNSR_Shadows_ReadHistory(history_uv);
			}
			shadow_previous_dbg = shadow_previous;

			shadow_clamped = clamp(shadow_previous, nmin, nmax);

			float const sigma = 20.0f;
			float const temporal_discontinuity = (shadow_previous - mean) / max(0.5f * std_deviation, 0.001f);
			temporal_discontinuity_dbg = temporal_discontinuity;
			float const sample_counter_damper = exp(-temporal_discontinuity * temporal_discontinuity / sigma);
			counter_damper_dbg = sample_counter_damper;
#if BYPASS_COUNTER_DAMPER
			(void)sample_counter_damper;
#else
			moments_current.z *= sample_counter_damper;
#endif

			if (moments_current.z < 16.0f)
			{
				const float variance_boost = max(16.0f - moments_current.z, 1.0f);
				variance = max(variance, spatial_variance);
				variance *= variance_boost;
			}
		}
		
		//const float history_weight = sqrt(max(8.0f - moments_current.z, 0.0f) / 8.0f);
		const float history_weight = sqrt(max(8.0f - moments_current.z, 0.0f) / 8.0f);
		shadow_clamped = lerp(shadow_clamped, shadow_current, lerp(0.05f, 1.0f, history_weight));

#if DEBUG_SHOW_HISTORY
		// Direct on-screen readout of the value read back from History.Shadow this frame.
		// variance=0 so the filter barely touches it -> what you see IS shadow_previous.
		//   - smooth/stable  -> History.Shadow feedback WORKS (bug is downstream: blend/clamp)
		//   - raw noise/black -> History.Shadow carries nothing (feedback/binding/first-frame broken)
		shadow_clamped = shadow_previous_dbg;
		variance = 0.0f;
#endif
#if DEBUG_SHOW_HISTORY_WEIGHT
		// Show the blend gate: bright = history-dominated (good), dark = current-dominated (jitter).
		shadow_clamped = 1.0f - lerp(0.05f, 1.0f, history_weight);
		variance = 0.0f;
#endif
#if DEBUG_SHOW_FIRSTFRAME
		// If this is bright (==1) every frame, bFirstFrame is stuck true and history is never read.
		shadow_clamped = (FFX_DNSR_Shadows_IsFirstFrame() != 0) ? 1.0f : 0.0f;
		variance = 0.0f;
#endif

#if SIMPLE_TEMPORAL
		// Clean baseline: the Welford running mean IS the denoised shadow.
		// moments_current.x = old_m + (shadow_current - old_m) / sample_count
		// For a static scene this converges to the true penumbra coverage per pixel.
		shadow_clamped = moments_current.x;
		// Keep variance modest so the spatial filter does light cleanup only.
		//variance = temporal_variance_pre_boost;
#endif
	}

#if DEBUG_VIZ == 0
	FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, float2(shadow_clamped, variance));
	FFX_DNSR_Shadows_WriteMoments(did.xy, moments_current);
#else
	// Debug visualizations - change DEBUG_VIZ to switch mode
	// All output to ReprojectionResult as float2; view in RenderDoc as R,G channels
	switch (DEBUG_VIZ)
	{
	case 1:
		// VELOCITY - R=vel.x*10, G=vel.y*10. Should be smooth gradients during camera move.
		// If noisy/static, velocity buffer is wrong. Values should be small (<0.5 for typical motion).
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, velocity * 10.0f);
		break;
	case 2:
		// DISOCCLUSION - R=1 if disoccluded (red=bad). Should only appear at screen edges during movement.
		// Large red regions = reprojection is broken = moments reset every frame = variance stuck at ~1
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, float2(is_disoccluded_dbg ? 1.0 : 0.0, history_uv_valid_dbg));
		break;
	case 3:
		// PREVIOUS MOMENTS - R=old_s (should be <1 for binary shadow), G=sample_count/16 (should grow to 1 over time).
		// If old_s > 1: garbage from history buffer = variance will explode.
		// If sample_count stays ~0: sample_counter_damper kills accumulation every frame.
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, float2(old_s_dbg, sample_count_dbg / 16.0f));
		break;
	case 4:
		// VARIANCE DECOMPOSITION - R=temporal variance before boost (should be <=0.25 for binary signal),
		// G=spatial variance (should be <=0.25). If R>>1, old_s from history is corrupted.
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, float2(temporal_variance_pre_boost, spatial_variance_dbg));
		break;
	case 5:
		// SAMPLE COUNT AFTER DAMPING + VARIANCE - R=moments_current.z/16 (should grow over time, 0-1),
		// G=variance (should be <=~1 normally). If R stays near 0 and G is huge, damper is killing accumulation.
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, float2(moments_current.z / 16.0f, min(variance, 16.0f) / 16.0f));
		break;
	case 6:
		// REPROJECTED HISTORY VALUE - R=shadow_previous (should be smooth/shadow-like, 0-1).
		// If noisy/bright/has edge artifacts, the history buffer or reprojection UV is wrong.
		// G=history_uv_valid (0 or 1) - if 0, the UV is out of bounds.
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, float2(shadow_previous_dbg, history_uv_valid_dbg));
		break;
	case 7:
		// RAW REPROJECTION UV - R=history_uv.x, G=history_uv.y. Should be 0-1 in screen.
		// If values go outside [0,1], velocity is wrong sign/magnitude.
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, history_uv);
		break;
	case 8:
		// DEPTH DIFFERENCE vs TOLERANCE - R=depth_difference * 10 (should be < 0.1 = dim),
		// G=depth_tolerance * 10 (0.01-0.1 range). If R>>G everywhere, reprojected depth
		// doesn't match previous depth = ReprojectionMatrix or depth buffer is wrong.
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, float2(depth_difference_dbg * 10.0f, depth_tolerance_dbg * 10.0f));
		break;
	case 9:
		// LINEAR DEPTHS - R=current linear depth from reprojection (clip_space.z linearized),
		// G=previous linear depth. Should be almost identical for static geometry.
		// If wildly different, ReprojectionMatrix is wrong or previous depth buffer is garbage.
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, float2(linear_depth_dbg / 100.0f, prev_linear_depth_dbg / 100.0f));
		break;
	case 10:
		// CLIP_SPACE_Z + Z_ALIGNMENT - R=clip_space.z after perspective divide (should be [0,1] or [-1,1] range),
		// G=z_alignment (0=looking along surface, 1=looking at surface head-on).
		// If clip_space.z is NaN/huge, the ReprojectionMatrix is producing garbage.
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, float2(abs(clip_space_z_dbg), z_alignment_dbg));
		break;
	case 11:
		// REPROJECTED NDC XY - R=reprojected_ndc_xy.x, G=reprojected_ndc_xy.y.
		// Should be in [-1,1]. If values are outside or always the same, the ReprojectionMatrix
		// is producing the wrong previous-frame screen position.
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, reprojected_ndc_xy_dbg * 0.5f + 0.5f);
		break;
	case 12:
		// CLIP SPACE W + RAW Z - R=clip_space.w before divide (should be ~1 for standard NDC input),
		// G=clip_space.z before divide. If w≈0, the perspective divide blows up z.
		// Should reveal if the matrix is singular or producing degenerate homogeneous coords.
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, float2(abs(clip_space_raw_dbg.w) * 0.1f, abs(clip_space_raw_dbg.z) * 0.1f));
		break;
	case 13:
		// TEMPORAL DISCONTINUITY + COUNTER DAMPER - R=abs(temporal_discontinuity) (should be <1 for stable),
		// G=counter_damper (should be close to 1 = no damping). If R>>1 and G<<1, the damper kills accumulation.
		// High discontinuity means shadow_previous differs a lot from spatial mean = bad history or wrong reprojection.
		FFX_DNSR_Shadows_WriteReprojectionResults(did.xy, float2(min(abs(temporal_discontinuity_dbg), 10.0f) / 10.0f, counter_damper_dbg));
		break;
	}
	FFX_DNSR_Shadows_WriteMoments(did.xy, moments_current);
#endif
}

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID, uint3 gid : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
	FFX_DNSR_Shadows_TileClassification(groupIndex, gid.xy);
}

#endif
