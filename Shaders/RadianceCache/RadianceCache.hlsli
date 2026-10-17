#pragma once

#ifndef RADIANCE_CACHE_SET
#define RADIANCE_CACHE_SET 3
#endif

#define SHARC_ENABLE_SH_ENCODING 1
#define SHARC_BLEND_ADJACENT_LEVELS 0 // Resolve writes in place; avoid cross-entry read/write races.
#define SHARC_SAMPLE_NUM_THRESHOLD 4
#include "../../Lib/SHARC/include/SharcCommon.h"

struct RadianceCacheConstants
{
	float4 CameraPositionScale;
	float4 PreviousCameraPosition;
	int4 Config;
	int4 Update;
	int4 Flags;
};

[[vk::binding(0, RADIANCE_CACHE_SET)]] RWStructuredBuffer<uint64_t> SharcHashEntries;
[[vk::binding(1, RADIANCE_CACHE_SET)]] RWStructuredBuffer<SharcAccumulationData> SharcAccumulation;
[[vk::binding(2, RADIANCE_CACHE_SET)]] RWStructuredBuffer<SharcPackedData> SharcResolved;
[[vk::binding(3, RADIANCE_CACHE_SET)]] ConstantBuffer<RadianceCacheConstants> RadianceCacheConfig;
[[vk::binding(4, RADIANCE_CACHE_SET)]] RWStructuredBuffer<uint> SharcStatistics;

SharcParameters GetRadianceCacheParameters()
{
	SharcParameters Cache;
	Cache.hashGridParameters.cameraPosition = RadianceCacheConfig.CameraPositionScale.xyz;
	Cache.hashGridParameters.logarithmBase  = SHARC_GRID_LOGARITHM_BASE;
	Cache.hashGridParameters.sceneScale     = RadianceCacheConfig.CameraPositionScale.w;
	Cache.hashGridParameters.levelBias      = SHARC_GRID_LEVEL_BIAS;

	Cache.hashGridData.capacity          = RadianceCacheConfig.Config.x;
	Cache.hashGridData.hashEntriesBuffer = SharcHashEntries;

	Cache.accumulationBuffer = SharcAccumulation;
	Cache.resolvedBuffer     = SharcResolved;
	Cache.radianceScale      = 1000.0f;
	return Cache;
}

SharcHitData MakeRadianceCacheHit(float3 Position, float3 Normal)
{
	SharcHitData Hit;
	Hit.positionWorld = Position;
	Hit.normalWorld   = Normal;

	Hit.radianceDirectionWorld  = Normal;
	Hit.radianceDirectionWeight = 0;
	return Hit;
}

// Entries contain diffuse irradiance / pi, before local albedo and emission.
// Sample thresholds and finite-value checks still apply, including for mirror rays.
bool QueryCachedDiffuseLighting(float3 Position, float3 Normal, out float3 Lighting)
{
	Lighting = 0;
	SharcParameters Cache = GetRadianceCacheParameters();
	SharcHitData Hit = MakeRadianceCacheHit(Position, Normal);
	float3 Value = 0;
	const bool Found = SharcGetCachedRadiance(Cache, Hit, Value, false) && all(isfinite(Value));

	if (RadianceCacheConfig.Flags.x != 0)
	{
		InterlockedAdd(SharcStatistics[0], 1);
		InterlockedAdd(SharcStatistics[Found ? 1 : 3], 1);
	}

	if (Found)
	{
		Lighting = max(Value, 0);
	}

	return Found;
}
