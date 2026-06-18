struct RayPayload
{
	float3 Colour;
	float3 Emissive;
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

#include "../RayTracingIrradianceVolumes.hlsli"

[[vk::binding(0, SET)]] RaytracingAccelerationStructure                     AccelerationStructure;
[[vk::binding(1, SET)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> Output;
[[vk::binding(2, SET)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> ResultDirectionDistance;
[[vk::binding(3, SET)]] [[vk::image_format("r16f")]]    RWTexture2D<float>  ResultRayPdf;
[[vk::binding(4, SET)]] Texture2D<float3>   GBufferAlbedo;
[[vk::binding(5, SET)]] Texture2D<float2>   GBufferNormals;
[[vk::binding(6, SET)]] Texture2D<float4>   GBufferRoughnessMetallic;
[[vk::binding(7, SET)]] Texture2D<float>    GBufferDepth;

[[vk::push_constant]]
struct _Params
{
	float4 CameraPosition;
	float  MaxRoughness;
	uint   Random;
	uint   UseRadianceCache;
} Params;

float RadicalInverseVdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10;
}

float2 ReflectionSampleSequence(uint2 pixel, uint frameIndex)
{
	uint pixelSeed = Random::Hash(pixel.x ^ Random::Hash(pixel.y));
	float2 sequence = float2(frac((frameIndex & 1023u) / 1024.0 + Random::FloatConstruct(pixelSeed)), RadicalInverseVdC(frameIndex ^ pixelSeed));
	return clamp(sequence, 1e-4, 0.9999);
}

[shader("raygeneration")]
void RayGen()
{
	const uint2 pixel = DispatchRaysIndex().xy;
	const uint2 launchDim = DispatchRaysDimensions().xy;

	float depth = GBufferDepth[pixel].x;
	// do not trace from sky, early exit
	if (IsSkyDepth(depth))
	{
		Output[pixel] = float4(0,0,0,-1);
		ResultDirectionDistance[pixel] = float4(0,0,0,-1);
		ResultRayPdf[pixel] = 0;
		return;
	}

	float2 RM = GBufferRoughnessMetallic[pixel].xy;

	// roughness cut, early exit
	if (RM.x > Params.MaxRoughness)
	{
		Output[pixel] = float4(0,0,0,-1);
		ResultDirectionDistance[pixel] = float4(0,0,0,-1);
		ResultRayPdf[pixel] = 0;
		return;
	}

	float3 Albedo = GBufferAlbedo[pixel].rgb;
	float3 Normal = NormalDecode(GBufferNormals[pixel]);
	float3 WP     = ReconstructWorldPositionFromDepth(pixel, depth, launchDim, GPUScene::GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix);

	float3 Direction = normalize(WP - GPUScene::GPUSceneScene[0].CameraCur.CameraPosition.xyz);

	BRDFData BRDF;
	BRDF.N         = Normal;
	BRDF.V         = -Direction;
	BRDF.Albedo    = Albedo;
	BRDF.Roughness = max(RM.r, 0.001);
	BRDF.Metallic  = RM.g;

	uint RngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + Params.Random);
	BRDFSample Sample = SampleBRDF_GGX(BRDF, ReflectionSampleSequence(pixel, Params.Random));
	Direction = Sample.Dir;
	BRDF.L = Direction;
	float RayPdf = max(Sample.Pdf, 1e-6);

	float3 RayAttenuation = EvaluateReflectionBRDF(BRDF);
	
	const float MaxDistance = 5000.0f;

	RayPayload payload;
	
	// ray trace
	{
		RayDesc Ray;
		Ray.Origin = WP + Normal * 0.03;
		Ray.TMin = 0.0;
		Ray.Direction = Direction;
		Ray.TMax = MaxDistance;

		TraceRay(AccelerationStructure, RAY_FLAG_FORCE_OPAQUE,
			~0, 0, 0, 0, Ray, payload);
	}

	float3 ColourResult = float3(0,0,0);

	// miss sky lighting
	if (payload.HitDistance < 0)
	{
		ColourResult = payload.Colour * RayAttenuation;
	}

	// hit point lighting
	{
		if (payload.HitDistance > 0)
		{
			BRDF.Albedo = payload.Colour;
			BRDF.N = payload.Normal;
			BRDF.V = -Direction;
			BRDF.Roughness = payload.RoughnessMetallic.x;
			BRDF.Metallic = payload.RoughnessMetallic.y;

			float3 HitPoint = WP + Direction * payload.HitDistance;
			HitPoint += BRDF.N * 0.01;
			float3 IndirectDiffuse = SampleRuntimeIrradianceVolumes(HitPoint, BRDF.N) * BRDF.Albedo;
			ColourResult = (payload.Emissive + RayTraceEvaluateDirectLighting(AccelerationStructure, HitPoint, RngState, BRDF) + IndirectDiffuse) * RayAttenuation;
		}
	}

	// write results
	{
		float denoiserRayLength        = max(payload.HitDistance, 0.0);
		Output[pixel]                  = float4(ColourResult, denoiserRayLength);
		ResultDirectionDistance[pixel] = float4(Direction, payload.HitDistance);
		ResultRayPdf[pixel]            = RayPdf;
	}
}

#endif // RAYGEN_SHADER

[shader("miss")]
void Miss(inout RayPayload payload)
{
	float3 Sun = normalize(GPUScene::GPUSceneScene[0].SunDirection.xyz);
	
	payload.Colour = Sky::Atmosphere(WorldRayOrigin(), WorldRayDirection(), Sun, GPUScene::GPUSceneScene[0].Sky);
	payload.Emissive = 0.0.xxx;
	payload.HitDistance = -1.0;
}

#include "../RayTracingClosestHit.hlsli"
