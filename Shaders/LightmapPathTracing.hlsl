struct RayPayload
{
	float3 Colour;
	float  HitDistance;
	float3 Normal;
	uint   ObjectId;
	float2 RoughnessMetallic;
};

#include "GPUScene.hlsli"
#include "Common.hlsli"
#include "SkyCommon.hlsli"

#ifdef RAYGEN_SHADER

#include "CommonRayTracing.hlsli"

#define SET 2

[[vk::binding(0, SET)]] RaytracingAccelerationStructure AccelerationStructure;
[[vk::binding(1, SET)]] Texture2D<float4>   PositionsImage;
[[vk::binding(2, SET)]] Texture2D<float4>   NormalsImage;
[[vk::binding(3, SET)]] Texture2D<float4>   ValidityImage;
[[vk::binding(4, SET)]] RWTexture2D<float4> LightmapOutput;

[[vk::push_constant]]
struct _Params
{
	int Random;
	int Bounces;
	int AccumulatedSamples;
	int RequestedSamples;
	int SamplesPerFrame;
} Parameters;

[shader("raygeneration")]
void RayGen()
{
	const int2 pixel = DispatchRaysIndex().xy;
	uint RngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + (Parameters.Random)); // Initial seed

	const float2 uv = float2(DispatchRaysIndex().xy) / float2(DispatchRaysDimensions().xy - 1);

	float3 origin = PositionsImage[pixel].xyz;
	float3 normal = NormalsImage[pixel].xyz;
	float  validity = ValidityImage[pixel].x;

	if (validity < 1)
	{
		// return;
	}

	float3 finalColor = float3(0,0,0);
	float3 previousColor = float3(0,0,0);

	if (Parameters.AccumulatedSamples != 0)
	{
		previousColor = LightmapOutput[pixel].rgb;
	}

	for (int i = 0; i < Parameters.SamplesPerFrame; i++)
	{
		float3 direction = Random::RandomDirectionHemisphere(Random::UniformDistrubition2d(RngState), normal);

		finalColor += RayTraceAccumulate(AccelerationStructure, origin, direction, Parameters.Bounces, RngState);
	}

	finalColor = finalColor + previousColor;

	if ((Parameters.AccumulatedSamples + Parameters.SamplesPerFrame) >= Parameters.RequestedSamples)
	{
		finalColor = finalColor / Parameters.RequestedSamples;
	}

	LightmapOutput[pixel] = float4(finalColor, 1);
}

#endif // RAYGEN_SHADER

[shader("miss")]
void Miss(inout RayPayload payload)
{
	float3 Sun = normalize(GPUScene::GPUSceneScene[0].SunDirection.xyz);
	
	payload.Colour = Sky::Atmosphere(WorldRayOrigin(), WorldRayDirection(), Sun, GPUScene::GPUSceneScene[0].Sky);
	payload.HitDistance = -1.0;
}

#include "RayTracingClosestHit.hlsli"