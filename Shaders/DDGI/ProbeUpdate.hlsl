struct RayPayload
{
	float3 Colour;
	float  HitDistance;
	float3 Normal;
	uint   ObjectId;
	float2 RoughnessMetallic;
    bool   IsFrontFace;
};

#define CLOSEST_HIT_REPORT_FACE

#include "../GPUScene.hlsli"
#include "../RayTracingClosestHit.hlsli"
#include "../SkyCommon.hlsli"

#ifdef RAYGEN_SHADER
#include "Defines.h"
#include "SDK/ddgi/include/DDGIRootConstants.hlsl"
#include "SDK/ddgi/Irradiance.hlsl"
#include "../CommonRayTracing.hlsli"

#define SET 2

[[vk::binding(0, SET)]] RaytracingAccelerationStructure SceneTLAS;
[[vk::binding(1, SET)]] StructuredBuffer<DDGIVolumeDescGPUPacked> DDGIVolumes;
[[vk::binding(2, SET)]] Texture2DArray<float4> ProbeData;
[[vk::binding(3, SET)]] RWTexture2DArray<float4> RayData;
[[vk::binding(4, SET)]] Texture2DArray<float4> ProbeIrradiance;
[[vk::binding(5, SET)]] Texture2DArray<float4> ProbeDistance;
[[vk::binding(6, SET)]] SamplerState BilinearSampler;

[shader("raygeneration")]
void RGS()
{
	// Get the DDGIVolume's index (from root/push constants)
	uint volumeIndex = GetDDGIVolumeIndex();

	// Get the DDGIVolume's constants from the structured buffer
	DDGIVolumeDescGPU volume = UnpackDDGIVolumeDescGPU(DDGIVolumes[volumeIndex]);

	// Compute the probe index for this thread
	int rayIndex = DispatchRaysIndex().x;                    // index of the ray to trace for this probe
	int probePlaneIndex = DispatchRaysIndex().y;             // index of this probe within the plane of probes
	int planeIndex = DispatchRaysIndex().z;                  // index of the plane this probe is part of
	int probesPerPlane = DDGIGetProbesPerPlane(volume.probeCounts);

	int probeIndex = (planeIndex * probesPerPlane) + probePlaneIndex;

	// Get the probe's grid coordinates
	float3 probeCoords = DDGIGetProbeCoords(probeIndex, volume);

	// Adjust the probe index for the scroll offsets
	probeIndex = DDGIGetScrollingProbeIndex(probeCoords, volume);

	// Get the probe's state
	float probeState = DDGILoadProbeState(probeIndex, ProbeData, volume);

	// Early out: do not shoot rays when the probe is inactive *unless* it is one of the "fixed" rays used by probe classification
	if (probeState == RTXGI_DDGI_PROBE_STATE_INACTIVE && rayIndex >= RTXGI_DDGI_NUM_FIXED_RAYS) return;

	// Get the probe's world position
	// Note: world positions are computed from probe coordinates *not* adjusted for infinite scrolling
	float3 probeWorldPosition = DDGIGetProbeWorldPosition(probeCoords, volume, ProbeData);

	// Get a random normalized ray direction to use for a probe ray
	float3 probeRayDirection = DDGIGetProbeRayDirection(rayIndex, volume);

	// Get the coordinates for the probe ray in the RayData texture array
	// Note: probe index is the scroll adjusted index (if scrolling is enabled)
	uint3 outputCoords = DDGIGetRayDataTexelCoords(rayIndex, probeIndex, volume);

	// Setup the probe ray
	RayDesc ray;
	ray.Origin = probeWorldPosition;
	ray.Direction = probeRayDirection;
	ray.TMin = 0.f;
	ray.TMax = volume.probeMaxRayDistance;

	// Setup the ray payload
	RayPayload payload = (RayPayload)0;

	// TODO:?
	// If classification is enabled, pass the probe's state to hit shaders through the payload
	//if(volume.probeClassificationEnabled) packedPayload.packed0.x = probeState;

	// Trace the Probe Ray
	TraceRay(
		SceneTLAS,
		RAY_FLAG_NONE,
		0xFF,
		0,
		0,
		0,
		ray,
		payload);

	// The ray missed. Store the miss radiance, set the hit distance to a large value, and exit early.
	if (payload.HitDistance < 0.f)
	{
		// Store the ray miss
		DDGIStoreProbeRayMiss(RayData, outputCoords, volume, payload.Colour);
		return;
	}

	// The ray hit a surface backface
	if (!payload.IsFrontFace)
	{
		// Store the ray backface hit
		DDGIStoreProbeRayBackfaceHit(RayData, outputCoords, volume, payload.HitDistance);
		return;
	}

	// Early out: a "fixed" ray hit a front facing surface. Fixed rays are not blended since their direction
	// is not random and they would bias the irradiance estimate. Don't perform lighting for these rays.
	if((volume.probeRelocationEnabled || volume.probeClassificationEnabled) && rayIndex < RTXGI_DDGI_NUM_FIXED_RAYS)
	{
		// Store the ray front face hit distance (only)
		//DDGIStoreProbeRayFrontfaceHit(RayData, outputCoords, volume, payload.HitDistance);
		//return;
	}

	// Direct Lighting and Shadowing
	//float3 diffuse = DirectDiffuseLighting(payload, GetGlobalConst(pt, rayNormalBias), GetGlobalConst(pt, rayViewBias), SceneTLAS, Lights);
	// TODO: Normal bias, View bias

	float3 hitWorldPosition = payload.HitDistance * ray.Direction + ray.Origin;
	BRDFData BRDF;
	BRDF.N = payload.Normal;
	BRDF.V = -ray.Direction;
	BRDF.Albedo = payload.Colour;
	BRDF.Roughness = payload.RoughnessMetallic.x;
	BRDF.Metallic = payload.RoughnessMetallic.y;

	uint RngState = 0; // TODO?

	float3 diffuse = RayTraceEvaluateDirectLighting(SceneTLAS, hitWorldPosition, RngState, BRDF);

	// Indirect Lighting (recursive)
	float3 irradiance = 0.f;
	float3 surfaceBias = DDGIGetSurfaceBias(payload.Normal, ray.Direction, volume);

	// TODO: sample recursive irradiance from other volumes
	
	// Get the volume resources needed for the irradiance query
	DDGIVolumeResources resources;
	resources.probeIrradiance = ProbeIrradiance;
	resources.probeDistance = ProbeDistance;
	resources.probeData = ProbeData;
	resources.bilinearSampler = BilinearSampler;

	// Compute volume blending weight
	float volumeBlendWeight = DDGIGetVolumeBlendWeight(hitWorldPosition, volume);

	// Don't evaluate irradiance when the surface is outside the volume
	if (volumeBlendWeight > 0)
	{
		// Get irradiance from the DDGIVolume
		irradiance = DDGIGetVolumeIrradiance(
			hitWorldPosition,
			surfaceBias,
			payload.Normal,
			volume,
			resources);

		// Attenuate irradiance by the blend weight
		irradiance *= volumeBlendWeight;
	}

	// Perfectly diffuse reflectors don't exist in the real world.
	// Limit the BRDF albedo to a maximum value to account for the energy loss at each bounce.
	float maxAlbedo = 0.9f;

	// Store the final ray radiance and hit distance
	float3 radiance = diffuse + ((min(payload.Colour, float3(maxAlbedo, maxAlbedo, maxAlbedo)) / PI) * irradiance);
	DDGIStoreProbeRayFrontfaceHit(RayData, outputCoords, volume, saturate(radiance), payload.HitDistance);

}
#endif

#ifdef MISS_SHADER
[shader("miss")]
void Miss(inout RayPayload payload)
{
	float3 Sun = normalize(float3(1, 1, 1)); // TODO: put sun direction into scene description
	
	payload.Colour = Sky::Atmosphere(WorldRayOrigin(), WorldRayDirection(), Sun, GPUScene::GPUSceneScene[0].Sky);
	payload.HitDistance = -1.0;
}
#endif