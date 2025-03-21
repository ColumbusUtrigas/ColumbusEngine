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

#include "../CommonRayTracing.hlsli"

#define SET 2

[[vk::push_constant]]
struct _Params
{
	float3 CameraPosition;
	uint Random;
	float DiffuseBoost;
	uint  UseRadianceCache;
	float UpscaleFactor;
} Params;

[[vk::binding(0, SET)]] RaytracingAccelerationStructure AccelerationStructure;
[[vk::binding(1, SET)]] RWTexture2D<float4> Output;
[[vk::binding(2, SET)]] Texture2D<float3>   GBufferNormals;
[[vk::binding(3, SET)]] Texture2D<float3>   GBufferWorldPosition;
[[vk::binding(4, SET)]] Texture2D<float>    GBufferDepth;

[shader("raygeneration")]
void RayGen()
{
	const int2 pixel = DispatchRaysIndex().xy;

	float depth = GBufferDepth[pixel].x;
	// do not trace from sky, early exit
	if (abs(depth) < EPSILON || abs(depth - 1) < EPSILON)
	{
		Output[pixel] = float4(0,0,0,0);
		return;
	}

	float3 Normal = GBufferNormals[pixel].xyz;
	float3 WP     = GBufferWorldPosition[pixel].xyz;

	uint RngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + (Params.Random)); // Initial seed

	BRDFSample Sample = SampleBRDF_Lambert(Normal, Random::UniformDistrubition2d(RngState));

	const float3 Direction = Sample.Dir;
	const float MaxDistance = 5000.0f;

	RayPayload payload;

	// ray trace
	{
		RayDesc Ray;
		Ray.Origin = WP;
		Ray.TMin = 0.01;
		Ray.Direction = Direction;
		Ray.TMax = MaxDistance;

		TraceRay(AccelerationStructure, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
			~0, 0, 0, 0, Ray, payload);
	}

	float3 Radiance = float3(0,0,0);

	// missed ray - hit the sky
	if (payload.HitDistance < 0)
	{
		Radiance += payload.Colour;
	}

	// evaluate lighting
	if (payload.HitDistance > 0)
	{
		BRDFData BRDF;
		BRDF.Albedo = payload.Colour;
		BRDF.N = payload.Normal;
		BRDF.V = -Direction;
		BRDF.Roughness = payload.RoughnessMetallic.x;
		BRDF.Metallic = payload.RoughnessMetallic.y;

		float3 HitPoint = WP + Direction * payload.HitDistance;
		Radiance = RayTraceEvaluateDirectLighting(AccelerationStructure, HitPoint, RngState, BRDF);
	}

	// final compute, write results
	{
		float NdotL = saturate(dot(Normal, Sample.Dir));
		float3 Irradiance = Radiance * LambertDiffuseBRDF(float3(1,1,1)) * NdotL / Sample.Pdf;

		// not physically correct but I've thought it's nice to have
		Irradiance *= Params.DiffuseBoost;

		Output[pixel] = float4(Irradiance, payload.HitDistance);
	}
}

#endif

[shader("miss")]
void Miss(inout RayPayload payload)
{
	float3 Sun = normalize(GPUScene::GPUSceneScene[0].SunDirection.xyz);
	
	payload.Colour = Sky::Atmosphere(WorldRayOrigin(), WorldRayDirection(), Sun, GPUScene::GPUSceneScene[0].Sky);
	payload.HitDistance = -1.0;
}

#include "../RayTracingClosestHit.hlsli"
