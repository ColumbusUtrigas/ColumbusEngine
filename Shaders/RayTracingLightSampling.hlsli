#pragma once

#include "Common.hlsli"
#include "GPUScene.hlsli"

struct RayTracedLightSample
{
	bool Valid;
	float3 Direction;
	float Distance;
	float3 Radiance;
};

RayTracedLightSample InvalidRayTracedLightSample()
{
	RayTracedLightSample Sample;
	Sample.Valid = false;
	Sample.Direction = float3(0, 0, 0);
	Sample.Distance = 0.0f;
	Sample.Radiance = float3(0, 0, 0);
	return Sample;
}

float RayTracedLightRangeAttenuation(float Distance, float Range)
{
	float SafeRange = max(Range, EPSILON);
	float Attenuation = saturate(1.0f - Distance * Distance / (SafeRange * SafeRange));
	return Attenuation * Attenuation;
}

float3 RayTracedLightConeDirection(float3 Direction, float ConeRadius, float2 Xi)
{
	Direction = normalize(Direction);

	if (ConeRadius <= 0.0f)
	{
		return Direction;
	}

	return normalize(Random::RandomDirectionCone(Direction, ConeRadius, Xi));
}

RayTracedLightSample MakeRayTracedLightSample(float3 Direction, float Distance, float3 Radiance)
{
	if (Distance <= EPSILON || any(isnan(Direction)) || any(isinf(Direction)))
	{
		return InvalidRayTracedLightSample();
	}

	RayTracedLightSample Sample;
	Sample.Valid = true;
	Sample.Direction = normalize(Direction);
	Sample.Distance = Distance;
	Sample.Radiance = Radiance;
	return Sample;
}

RayTracedLightSample SampleRayTracedLight(GPULight Light, float3 Origin, float2 Xi)
{
	switch (Light.Type)
	{
	case GPULIGHT_DIRECTIONAL:
	{
		float3 Direction = RayTracedLightConeDirection(Light.Direction.xyz, Light.SourceRadius, Xi);
		return MakeRayTracedLightSample(Direction, 5000.0f, Light.Color.rgb);
	}
	case GPULIGHT_POINT:
	case GPULIGHT_SPOT:
	{
		float3 ToLight = Light.Position.xyz - Origin;
		float Distance = length(ToLight);
		if (Distance <= EPSILON || Distance > Light.Range)
		{
			return InvalidRayTracedLightSample();
		}

		float3 CenterDirection = ToLight / Distance;
		float Attenuation = RayTracedLightRangeAttenuation(Distance, Light.Range);

		if (Light.Type == GPULIGHT_SPOT)
		{
			float Angle = saturate(dot(CenterDirection, Light.Direction.xyz));
			Attenuation *= smoothstep(Light.SizeOrSpotAngles.y, Light.SizeOrSpotAngles.x, Angle);
		}

		if (Attenuation <= 0.0f)
		{
			return InvalidRayTracedLightSample();
		}

		float ConeRadius = Light.SourceRadius / max(Distance, EPSILON);
		float3 Direction = RayTracedLightConeDirection(CenterDirection, ConeRadius, Xi);
		return MakeRayTracedLightSample(Direction, Distance, Light.Color.rgb * Attenuation);
	}
	case GPULIGHT_RECTANGLE:
	{
		float3x3 LightAxis = ComputeTangentsFromVector(Light.Direction.xyz);
		float2 HalfSize = Light.SizeOrSpotAngles;
		float3 Ex = LightAxis[0] * HalfSize.x;
		float3 Ey = LightAxis[1] * HalfSize.y;
		float2 Offset = Xi * 2.0f - 1.0f;
		float3 SamplePoint = Light.Position.xyz + Offset.x * Ex + Offset.y * Ey;
		float3 ToLight = SamplePoint - Origin;
		return MakeRayTracedLightSample(ToLight, length(ToLight), Light.Color.rgb);
	}
	case GPULIGHT_DISC:
	{
		float3x3 LightAxis = ComputeTangentsFromVector(Light.Direction.xyz);
		float Radius = sqrt(Xi.x);
		float Phi = TWO_PI * Xi.y;
		float2 Disk = float2(cos(Phi), sin(Phi)) * Radius;
		float2 HalfSize = Light.SizeOrSpotAngles;
		float3 SamplePoint = Light.Position.xyz + LightAxis[0] * (Disk.x * HalfSize.x) + LightAxis[1] * (Disk.y * HalfSize.y);
		float3 ToLight = SamplePoint - Origin;
		return MakeRayTracedLightSample(ToLight, length(ToLight), Light.Color.rgb);
	}
	default:
		return InvalidRayTracedLightSample();
	}
}
