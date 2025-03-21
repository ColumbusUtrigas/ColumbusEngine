struct RayPayload
{
	float3 Colour;
	float  HitDistance;
	float3 Normal;
	uint   ObjectId;
	float2 RoughnessMetallic;
};

#include "../GPUScene.hlsli"
#include "../Common.hlsli"
#include "../SkyCommon.hlsli"

#ifdef RAYGEN_SHADER

	#define RADIANCE_CACHE_BINDING 1
	#define RADIANCE_CACHE_SET 2
	#include "RadianceCache.hlsli"

	#include "../CommonRayTracing.hlsli"

	#define SET 2

	[[vk::binding(0, SET)]] RaytracingAccelerationStructure AccelerationStructure;

	[[vk::push_constant]]
	struct _Params
	{
		float3 CameraPosition;
		int Random;
	} Params;

	int3 Probe3dIndex(uint FlatId)
	{
		return int3(
			FlatId % CASCADE_RESOLUTION,
			(FlatId / CASCADE_RESOLUTION) % CASCADE_RESOLUTION,
			FlatId / (CASCADE_RESOLUTION * CASCADE_RESOLUTION)
		);
	}

	float3 CalcProbePosition(float ProbeSpacing, int3 ProbeIndex)
	{
		float3 CascadeBounds = ProbeSpacing * float3(NUM_CASCADES, NUM_CASCADES, NUM_CASCADES);
		return ProbeIndex * ProbeSpacing + Params.CameraPosition - (CascadeBounds / 2);
	}

	[shader("raygeneration")]
	void RayGen()
	{
		const int2 pixel = DispatchRaysIndex().xy;
		const float2 ndc = (float2(pixel) / float2(DispatchRaysDimensions().xy) * 2 - 1) * float2(1, -1);

		uint RngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + (Params.Random)); // Initial seed

		const uint Cascade = DispatchRaysIndex().x;
		const uint Probe   = DispatchRaysIndex().y;

		const float  ProbeSpacing  = CalcCascadeProbeSpacing(Cascade);
		const float3 CascadeCenter = Params.CameraPosition; // TODO: improve
		const int3   ProbeIndex    = Probe3dIndex(Probe);
		const uint   ProbeFlatId   = CascadeIndexOffset(Cascade) + Probe;
		const float3 ProbePosition = CalcProbePosition(ProbeSpacing, ProbeIndex);

		const float3 Direction = Random::RandomDirectionSphere(Random::UniformDistrubition2d(RngState));

		const int Bounces = 5;
		const float3 Radiance = RayTraceAccumulate(AccelerationStructure, ProbePosition, Direction, Bounces, RngState);
		
		RadianceCacheBuffer[ProbeFlatId].Irradiance = lerp(RadianceCacheBuffer[ProbeFlatId].Irradiance, float4(Radiance, 1), 0.1);
	}

#endif // RAYGEN_SHADER

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.Colour = float3(0,0,0);
	payload.HitDistance = -1.0;
}

#include "../RayTracingClosestHit.hlsli"