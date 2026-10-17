struct RayPayload
{
	float3 Colour;
	float3 Emissive;
	float  HitDistance;
	float3 Normal;
	uint   ObjectId;
	float2 RoughnessMetallic;
};

#if USE_SHARC
#define SHARC_UPDATE 0
#define SHARC_QUERY 1
#include "../RadianceCache/RadianceCache.hlsli"
#endif

#include "../GPUScene.hlsli"
// GI hit lighting retains the miss fast path; cached GI casts no visibility rays.
#include "../RayTracingShadowVisibility.hlsli"
#include "../Common.hlsli"
#include "../SkyCommon.hlsli"

#ifdef RAYGEN_SHADER

#include "../CommonRayTracing.hlsli"

#define SET 2

#include "../RayTracingIrradianceVolumes.hlsli"

[[vk::push_constant]]
struct _Params
{
	float3 CameraPosition;
	uint Random;
	float DiffuseBoost;
	uint  UseRadianceCache;
	float UpscaleFactor;
} Params;

[[vk::binding(0, SET)]] RaytracingAccelerationStructure                     AccelerationStructure;
[[vk::binding(1, SET)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> Output;
[[vk::binding(2, SET)]] Texture2D<float2>                                      GBufferNormals;
[[vk::binding(3, SET)]] Texture2D<float>                                       GBufferDepth;
[[vk::binding(4, SET)]] Texture2D<float4>                                      GBufferSourcePixel;

[shader("raygeneration")]
void RayGen()
{
	const uint2 pixel = DispatchRaysIndex().xy;

	float depth = GBufferDepth[pixel].x;
	// do not trace from sky, early exit
	if (IsSkyDepth(depth))
	{
		Output[pixel] = float4(0,0,0,0);
		return;
	}

	float3 Normal = NormalDecode(GBufferNormals[pixel]);
	int2 sourcePixel = clamp(int2(round(GBufferSourcePixel[pixel].xy)), int2(0, 0), GPUScene::GPUSceneScene[0].RenderSize - 1);
	float3 WP = ReconstructWorldPositionFromDepth((uint2)sourcePixel, depth, (uint2)GPUScene::GPUSceneScene[0].RenderSize, GPUScene::GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix);

	uint RngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + (Params.Random)); // Initial seed

	BRDFSample Sample = SampleBRDF_Lambert(Normal, Random::UniformDistrubition2d(RngState));

	const float3 Direction = Sample.Dir;
	const float MaxDistance = 5000.0f;

	RayPayload payload;

	const float3 RayOrigin = WP + Normal * max(0.002f, 0.00002f * distance(WP, GPUScene::GPUSceneScene[0].CameraCur.CameraPosition.xyz));

	// ray trace
	{
		RayDesc Ray;
		float RayBias = max(0.002, 0.00002 * distance(WP, GPUScene::GPUSceneScene[0].CameraCur.CameraPosition.xyz));
		Ray.Origin = RayOrigin;
		Ray.TMin = RayBias;
		Ray.Direction = Direction;
		Ray.TMax = MaxDistance;

		TraceRay(AccelerationStructure, RAY_FLAG_FORCE_OPAQUE,
			~0, 0, 0, 0, Ray, payload);
	}

	float3 Radiance = float3(0,0,0);

	// missed ray - hit the sky
	if (payload.HitDistance < 0)
	{
		Radiance += payload.Colour;
	}

	if (payload.HitDistance > 0)
	{
#if USE_SHARC
		Radiance = payload.Emissive + payload.Colour;
#else
		BRDFData BRDF;
		BRDF.Albedo    = payload.Colour;
		BRDF.N         = payload.Normal;
		BRDF.V         = -Direction;
		BRDF.Roughness = payload.RoughnessMetallic.x;
		BRDF.Metallic  = payload.RoughnessMetallic.y;

		const float3 HitPoint = RayOrigin + Direction * payload.HitDistance;
		const float3 IndirectDiffuse = SampleRuntimeIrradianceVolumes(HitPoint, BRDF.N) * BRDF.Albedo;
		Radiance = payload.Emissive + RayTraceEvaluateDirectLighting(AccelerationStructure, HitPoint, RngState, BRDF) + IndirectDiffuse;
#endif
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
	if (IsOpaqueShadowRay())
	{
		payload.HitDistance = -1.0f;
		return;
	}

	float3 Sun = normalize(GPUScene::GPUSceneScene[0].SunDirection.xyz);
	
	payload.Colour = Sky::Atmosphere(WorldRayOrigin(), WorldRayDirection(), Sun, GPUScene::GPUSceneScene[0].Sky);
	payload.Emissive = 0.0.xxx;
	payload.HitDistance = -1.0;
}

#include "../RayTracingClosestHit.hlsli"
