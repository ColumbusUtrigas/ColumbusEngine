struct RayPayload
{
	float3 Colour;
	float3 Emissive;
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
[[vk::binding(1, IRRADIANCE_PROBES_SET)]] RWStructuredBuffer<IrradianceFixedRayData> IrradianceFixedRayDataBuffer;

[[vk::push_constant]]
struct _Params
{
	int SamplesPerProbe;
	int Bounces;
	int Random;
	int Mode; // 0 fixed-ray classification trace, 1 ambient cube trace, 2 visibility distance trace.
	int RayDataStride;
	int ProbeOffset;
	int ProbeCount;
	int SampleOffset;
	int TotalSamplesPerProbe;
} Params;

[shader("raygeneration")]
void RGS()
{
	if (Params.Mode == 0)
	{
		uint RayIndex = DispatchRaysIndex().x;
		uint LocalProbeIndex = DispatchRaysIndex().y;
		if (LocalProbeIndex >= Params.ProbeCount)
			return;

		uint ProbeIndex = Params.ProbeOffset + LocalProbeIndex;
		uint RayDataIndex = ProbeIndex * Params.RayDataStride + RayIndex;

		RayDesc Ray;
		Ray.Origin = IrradianceProbes[ProbeIndex].PositionState.xyz;
		Ray.Direction = IrradianceFixedRayDirection(RayIndex);
		Ray.TMin = 0.0;
		Ray.TMax = 5000.0;

		RayPayload Payload = (RayPayload)0;
		TraceRay(SceneTLAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, Ray, Payload);

		float Distance = 1e27;
		if (Payload.HitDistance >= 0.0)
		{
			Distance = Payload.IsFrontFace ? Payload.HitDistance : -Payload.HitDistance * 0.2;
		}

		IrradianceFixedRayDataBuffer[RayDataIndex].Distance = Distance;
		return;
	}

	if (Params.Mode == 2)
	{
		uint RayIndex = DispatchRaysIndex().x;
		uint LocalProbeIndex = DispatchRaysIndex().y;
		if (LocalProbeIndex >= Params.ProbeCount)
			return;

		uint ProbeIndex = Params.ProbeOffset + LocalProbeIndex;
		uint RayDataIndex = ProbeIndex * Params.RayDataStride + RayIndex;

		RayDesc Ray;
		Ray.Origin = IrradianceProbes[ProbeIndex].PositionState.xyz;
		Ray.Direction = IrradianceSphericalFibonacci(RayIndex, Params.SamplesPerProbe);
		Ray.TMin = 0.0;
		Ray.TMax = 5000.0;

		RayPayload Payload = (RayPayload)0;
		TraceRay(SceneTLAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, Ray, Payload);

		float Distance = Payload.HitDistance >= 0.0 ? Payload.HitDistance : 1e27;
		IrradianceFixedRayDataBuffer[RayDataIndex].Distance = Distance;
		return;
	}

	uint LocalProbeIndex = DispatchRaysIndex().x;
	if (LocalProbeIndex >= Params.ProbeCount)
		return;

	uint ProbeIndex = Params.ProbeOffset + LocalProbeIndex;
	if (IrradianceProbes[ProbeIndex].PositionState.w < 0.5)
	{
		if (Params.SampleOffset == 0)
		{
			[unroll]
			for (int LobeIndex = 0; LobeIndex < 6; LobeIndex++)
			{
				IrradianceProbes[ProbeIndex].Irradiance[LobeIndex] = float4(0, 0, 0, 1);
			}
		}
		return;
	}

	int SamplesPerLobe = max(Params.SamplesPerProbe, 1);
	int TotalSamplesPerLobe = max(Params.TotalSamplesPerProbe, 1);
	bool IsFirstSampleChunk = Params.SampleOffset == 0;
	bool IsLastSampleChunk = Params.SampleOffset + SamplesPerLobe >= TotalSamplesPerLobe;
	float3 Origin = IrradianceProbes[ProbeIndex].PositionState.xyz;

	[unroll]
	for (int LobeIndex = 0; LobeIndex < 6; LobeIndex++)
	{
		float3 Irradiance = IsFirstSampleChunk ? float3(0, 0, 0) : IrradianceProbes[ProbeIndex].Irradiance[LobeIndex].rgb;
		for (int SampleIndex = 0; SampleIndex < SamplesPerLobe; SampleIndex++)
		{
			int GlobalSampleIndex = Params.SampleOffset + SampleIndex;
			if (GlobalSampleIndex >= TotalSamplesPerLobe)
				continue;

			uint rngState = Random::Hash(ProbeIndex ^ Params.Random ^ (LobeIndex * 0x9E3779B9u) ^ (GlobalSampleIndex * 0x85EBCA6Bu));
			float3 Direction = Random::RandomDirectionHemisphere(Random::UniformDistrubition2d(rngState), IRRADIANCE_BASIS[LobeIndex]);
			Irradiance += RayTraceAccumulate(SceneTLAS, Origin, Direction, Params.Bounces, rngState);
		}

		if (IsLastSampleChunk)
		{
			Irradiance /= TotalSamplesPerLobe;
		}

		IrradianceProbes[ProbeIndex].Irradiance[LobeIndex] = float4(Irradiance, 1.0);
	}
}
#endif

#ifdef MISS_SHADER
[shader("miss")]
void Miss(inout RayPayload payload)
{
	float3 Sun = normalize(GPUScene::GPUSceneScene[0].SunDirection.xyz);
	
	payload.Colour = Sky::Atmosphere(WorldRayOrigin(), WorldRayDirection(), Sun, GPUScene::GPUSceneScene[0].Sky);
	payload.Emissive = 0.0.xxx;
	payload.HitDistance = -1.0;
}
#endif
