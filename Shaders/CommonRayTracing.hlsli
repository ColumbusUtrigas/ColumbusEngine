#pragma once

#ifndef PAYLOAD_TYPE
#define PAYLOAD_TYPE RayPayload
#endif

#include "BRDF.hlsli"
#include "Common.hlsli"
#include "GPUScene.hlsli"

float3 SanitizeRadiance(float3 Value)
{
	if (any(isnan(Value)) || any(isinf(Value)))
		return float3(0, 0, 0);

	return max(Value, 0.0.xxx);
}

float3 ClampPathWeight(float3 Weight, float MaxComponent)
{
	Weight = SanitizeRadiance(Weight);
	float Peak = max(Weight.x, max(Weight.y, Weight.z));
	if (Peak > MaxComponent)
	{
		Weight *= MaxComponent / Peak;
	}
	return Weight;
}

float Luminance(float3 Value)
{
	return dot(Value, float3(0.2126, 0.7152, 0.0722));
}

float3 SampleConeRay(float3 Direction, float BaseRadius, float2 Random)
{
	// generate points in circle
	float theta = Random.x * 2 * PI;
	float radius = Random.y * 0.5 * BaseRadius;
	float2 circle = float2(cos(theta) * radius, sin(theta) * radius);

	// generate cone basis
	// TODO: verify handinness
	float3 up = Direction.y < 0.999 ? float3(0, 1, 0) : float3(0, 0, 1);
	float3 right = normalize(cross(up, Direction));
	float3 forward = normalize(cross(right, up));

	// use basis to transform points
	return Direction + circle.x * right + circle.y * forward;
}

// returns 0 if occluded, 1 if not
float TraceShadowRay(const in RaytracingAccelerationStructure AS, float3 Origin, float3 Direction, float MaxDist)
{
	PAYLOAD_TYPE payload;
	RayDesc Ray;
	Ray.Origin = Origin;
	Ray.TMin = 0;
	Ray.Direction = Direction;
	Ray.TMax = MaxDist;
	TraceRay(AS, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, ~0, 0, 0, 0, Ray, payload);
	
	return payload.HitDistance > 0 ? 0.0f : 1.0f;
}

float3 SampleDirectionalLight(const in RaytracingAccelerationStructure AS, GPULight Light, float3 origin, float3 normal, float2 Random)
{
	float3 direction = normalize(Light.Direction.xyz);
	float occlusion = TraceShadowRay(AS, origin, direction, 5000.0f);
	return Light.Color.rgb * occlusion;
}

float3 SamplePointLight(const in RaytracingAccelerationStructure AS, GPULight Light, float3 origin, float3 normal, float2 Random)
{
	float3 direction = normalize(Light.Position.xyz - origin);
	float dist = distance(Light.Position.xyz, origin);

	// float attenuation = 1.0 / (1.0 + dist);
	float attenuation = clamp(1.0 - dist * dist / (Light.Range * Light.Range), 0.0, 1.0);
	attenuation *= attenuation;
	
	// sample shadow
	float occlusion = TraceShadowRay(AS, origin, direction, dist);

	return attenuation * Light.Color.rgb * occlusion;
}

float3 RayTraceEvaluateDirectLighting(const in RaytracingAccelerationStructure AS, float3 Origin, uint RngState, BRDFData BRDF)
{
	float3 Result = float3(0, 0, 0);
	
	// TODO: sample from a grid
	for (uint l = 0; l < GPUScene::GetLightsCount(); l++)
	{
		float3 LightSample = float3(0, 0, 0);

		// TODO: get it from sample functions
		BRDF.L = float3(0, 0, 0); // light direction

		float2 Xi = Random::UniformDistrubition2d(RngState);
		// TODO: light PDF, as every time we do a random decision, we need to weight it by it's probability
		
        GPULight Light = GPUScene::GPUSceneLights[l];
		
        float Distance = distance(Light.Position.xyz, Origin);
		
		// TODO: unify it with code path in GBufferLightingPass.hlsl

		switch (Light.Type)
		{
		case GPULIGHT_DIRECTIONAL:
			LightSample = SampleDirectionalLight(AS, Light, Origin, BRDF.N, Xi);
			BRDF.L = Light.Direction.xyz;
			break;
		case GPULIGHT_POINT:
			LightSample = SamplePointLight(AS, Light, Origin, BRDF.N, Xi);
			// TODO: account for sphere light
			BRDF.L = normalize(Light.Position.xyz - Origin);
			break;
		case GPULIGHT_SPOT:
            LightSample = SamplePointLight(AS, Light, Origin, BRDF.N, Xi);
            BRDF.L = normalize(Light.Position.xyz - Origin);

            float angle = saturate(dot(BRDF.L, Light.Direction.xyz));
            float2 angles = Light.SizeOrSpotAngles;

            LightSample *= smoothstep(angles.y, angles.x, angle);
            break;
			
			// TODO: area lights
        }

        Result += SanitizeRadiance(EvaluateBRDFCos(BRDF) * LightSample);
    }
	
	return SanitizeRadiance(Result);
}

// performs multi-bounce raytracing and path accumulation
float3 RayTraceAccumulate(const in RaytracingAccelerationStructure AS,
	float3 Origin, float3 Direction, int MaxBounces, inout uint RngState
)
{
	float3 PathRadiance = float3(0, 0, 0);
	float3 PathAttenuation = float3(1, 1, 1);
	BRDFData BRDF;
	
	PAYLOAD_TYPE payload;

	// always at least one ray
	for (int i = 0; i <= MaxBounces; i++)
	{
		RayDesc Ray;
		Ray.Origin = Origin;
		Ray.TMin = 0;
		Ray.Direction = Direction;
		Ray.TMax = 5000;
		TraceRay(AS, RAY_FLAG_NONE, ~0, 0, 0, 0, Ray, payload);

		if (payload.HitDistance > 0)
		{
			BRDF.Albedo = payload.Colour;
			BRDF.N = payload.Normal;
			BRDF.V = -Direction;
			BRDF.Roughness = payload.RoughnessMetallic.x;
			BRDF.Metallic = payload.RoughnessMetallic.y;

			float3 HitPoint = payload.HitDistance * Direction + Origin;
#ifdef PAYLOAD_HAS_GEOMETRIC_NORMAL
			Origin = HitPoint + payload.GeometricNormal * 0.001;
#else
			Origin = HitPoint + BRDF.N * 0.001;
#endif

			PathRadiance += SanitizeRadiance(payload.Emissive * PathAttenuation);

			// NEE
			PathRadiance += SanitizeRadiance(RayTraceEvaluateDirectLighting(AS, Origin, RngState, BRDF) * PathAttenuation);

			// generate new ray and apply BRDF to the segment
			{
				float3 F0 = BRDFF0(BRDF);
				float SpecularWeight = max(Luminance(F0), 0.05f);
				float DiffuseWeight = max((1.0 - BRDF.Metallic) * Luminance(BRDF.Albedo), 0.05f);
				float WeightSum = SpecularWeight + DiffuseWeight;
				float SpecularProb = SpecularWeight / WeightSum;
				float DiffuseProb = DiffuseWeight / WeightSum;

				BRDFSample Sample;
				float Selector = Random::StepAndOutputRNGFloat(RngState);
				if (Selector < DiffuseProb)
				{
					Sample = SampleDiffuseBRDF(BRDF, Random::UniformDistrubition2d(RngState));
				}
				else
				{
					Sample = SampleBRDF_GGX(BRDF, Random::UniformDistrubition2d(RngState));
				}

				BRDF.L = Sample.Dir;
				Direction = Sample.Dir;

				float NdotL = max(0, dot(BRDF.N, BRDF.L));
				if (NdotL <= 0.0 || !isfinite(Sample.Pdf) || Sample.Pdf <= 1e-5 || any(isnan(Direction)) || any(isinf(Direction)))
					break;

				float TotalPdf = DiffuseProb * DiffusePDF(BRDF, BRDF.L) + SpecularProb * BRDFCalcPDF(BRDF);
				if (!isfinite(TotalPdf) || TotalPdf <= 1e-5)
					break;

				float3 Weight = EvaluateBRDFCos(BRDF) / TotalPdf;
				PathAttenuation *= ClampPathWeight(Weight, 8.0);
				PathAttenuation = ClampPathWeight(PathAttenuation, 16.0);
            }
		}
		else // sky
		{
			PathRadiance += SanitizeRadiance(payload.Colour.rgb * PathAttenuation);
			break;
		}
	}
	
	return SanitizeRadiance(PathRadiance);
}
