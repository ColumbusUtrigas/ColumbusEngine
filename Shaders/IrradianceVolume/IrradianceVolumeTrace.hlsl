struct RayPayload
{
	float3 Colour;
	float HitDistance;
	float3 Normal;
	uint ObjectId;
	float2 RoughnessMetallic;
	bool IsFrontFace;
};

#define CLOSEST_HIT_REPORT_FACE

#include "../GPUScene.hlsli"
#include "../RayTracingClosestHit.hlsli"
#include "../SkyCommon.hlsli"

#ifdef RAYGEN_SHADER
#define SET 2
#define IRRADIANCE_PROBES_SET 3

#include "IrradianceVolumeCommon.hlsli"
#include "../CommonRayTracing.hlsli"

[[vk::binding(0, SET)]] RaytracingAccelerationStructure SceneTLAS;

#define ACCESS_PROBE() IrradianceProbes[DispatchRaysIndex().x]

[[vk::push_constant]]
struct _Params {
	int SamplesPerProbe;
	int Bounces;
	int Random;
} Params;

[shader("raygeneration")]
void RGS()
{
	uint rngState = DispatchRaysIndex().x + DispatchRaysIndex().y + Params.Random;  // Initial seed

	// null all the probes
	for (int i = 0; i < 6; i++)
	{
		ACCESS_PROBE().Irradiance[i] = float3(0, 0, 0);
	}
	
    float3 origin = ACCESS_PROBE().Position;

	for (int i = 0; i < 6; i++)
	{
		for (int s = 0; s < Params.SamplesPerProbe; s++)
		{
			float3 direction = Random::RandomDirectionHemisphere(Random::UniformDistrubition2d(rngState), IRRADIANCE_BASIS[i]);
            float3 color = RayTraceAccumulate(SceneTLAS, origin, direction, Params.Bounces, rngState);

			ACCESS_PROBE().Irradiance[i] += color;
		}
		
		ACCESS_PROBE().Irradiance[i] /= Params.SamplesPerProbe;
	}
}
#endif

#ifdef MISS_SHADER
[shader("miss")]
void Miss(inout RayPayload payload)
{
	float3 Sun = normalize(float3(1, 1, 1)); // TODO: put sun direction into scene description
	
	payload.Colour = Sky::Atmosphere(WorldRayOrigin(), WorldRayDirection(), Sun);
	payload.HitDistance = -1.0;
}
#endif