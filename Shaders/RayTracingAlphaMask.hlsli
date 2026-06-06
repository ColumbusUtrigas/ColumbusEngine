#pragma once

#ifndef PAYLOAD_TYPE
#define PAYLOAD_TYPE RayPayload
#endif

#include "GPUScene.hlsli"

bool PayloadRejectsAlphaMaskedHit(PAYLOAD_TYPE Payload)
{
	return Payload.ShadingMode == MATERIAL_SHADING_MASKED && Payload.Alpha < Payload.AlphaCutoff;
}

void TraceClosestHitSkippingAlphaMask(const in RaytracingAccelerationStructure AS, inout PAYLOAD_TYPE Payload, float3 Origin, float3 Direction, float MaxDist, uint RayFlags, bool UseAlphaMask)
{
	float travelled = 0.0f;
	float3 rayOrigin = Origin;

	[loop]
	for (int i = 0; i < 16; i++)
	{
		RayDesc Ray;
		Ray.Origin = rayOrigin;
		Ray.TMin = 0.0f;
		Ray.Direction = Direction;
		Ray.TMax = MaxDist - travelled;

		TraceRay(AS, RayFlags, ~0, 0, 0, 0, Ray, Payload);

		if (Payload.HitDistance <= 0.0f || !UseAlphaMask || !PayloadRejectsAlphaMaskedHit(Payload))
		{
			if (Payload.HitDistance > 0.0f)
			{
				Payload.HitDistance += travelled;
			}
			return;
		}

		float step = Payload.HitDistance + 0.001f;
		travelled += step;
		if (travelled >= MaxDist)
		{
			Payload.HitDistance = -1.0f;
			return;
		}

		rayOrigin = Origin + Direction * travelled;
	}
}

float TraceShadowRayWithAlphaMask(const in RaytracingAccelerationStructure AS, float3 Origin, float3 Direction, float MaxDist, uint RayFlags, bool UseAlphaMask)
{
	PAYLOAD_TYPE payload;
	TraceClosestHitSkippingAlphaMask(AS, payload, Origin, Direction, MaxDist, RayFlags, UseAlphaMask);
	return payload.HitDistance > 0 ? 0.0f : 1.0f;
}
