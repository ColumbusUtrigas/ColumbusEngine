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

[[vk::binding(0, SET)]] RaytracingAccelerationStructure AccelerationStructure;
[[vk::binding(1, SET)]] RWTexture2D<float4> Output;
[[vk::binding(2, SET)]] RWTexture2D<float4> ResultDirectionDistance;
[[vk::binding(3, SET)]] Texture2D<float3>   GBufferAlbedo;
[[vk::binding(4, SET)]] Texture2D<float3>   GBufferNormals;
[[vk::binding(5, SET)]] Texture2D<float3>   GBufferWorldPosition;
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


[shader("raygeneration")]
void RayGen()
{
	const int2 pixel = DispatchRaysIndex().xy;

	float depth = GBufferDepth[pixel].x;
	// do not trace from sky, early exit
	if (abs(depth) < EPSILON || abs(depth - 1) < EPSILON)
	{
		Output[pixel] = float4(0,0,0,0);
		ResultDirectionDistance[pixel] = float4(0,0,0,-1);
		return;
	}

	float2 RM = GBufferRoughnessMetallic[pixel].xy;

	// roughness cut, early exit
	if (RM.x > Params.MaxRoughness)
	{
		Output[pixel] = float4(0,0,0,0);
		ResultDirectionDistance[pixel] = float4(0,0,0,-1);
		return;
	}

	float3 Albedo = GBufferAlbedo[pixel].rgb;
	float3 Normal = GBufferNormals[pixel].xyz;
	float3 WP     = GBufferWorldPosition[pixel].xyz;

	float3 Direction = normalize(WP - GPUScene::GPUSceneScene[0].CameraCur.CameraPosition.xyz);

	BRDFData BRDF;
	BRDF.N         = Normal;
	BRDF.V         = -Direction;
	BRDF.Albedo    = Albedo;
	BRDF.Roughness = RM.r;
	BRDF.Metallic  = RM.g;

	uint RngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + (Params.Random)); // Initial seed

	BRDFSample Sample = SampleBRDF_GGX(BRDF, Random::UniformDistrubition2d(RngState));
	Direction = Sample.Dir;
	BRDF.L = Direction;

	float NdotL = saturate(dot(BRDF.N, BRDF.L));
	float3 RayAttenuation = EvaluateBRDF(BRDF, float3(1,1,1)) / max(Sample.Pdf, 0.001);
	
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

	float3 ColourResult = float3(0,0,0);

	// miss sky lighting
	if (payload.HitDistance < 0)
	{
		ColourResult = payload.Colour;
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
			ColourResult = RayTraceEvaluateDirectLighting(AccelerationStructure, HitPoint, RngState, BRDF) * RayAttenuation;
		}
	}

	// write results
	{
		Output[pixel]                  = float4(ColourResult, 1);
		ResultDirectionDistance[pixel] = float4(Direction, payload.HitDistance);
	}
}

#endif // RAYGEN_SHADER

[shader("miss")]
void Miss(inout RayPayload payload)
{
	float3 Sun = normalize(GPUScene::GPUSceneScene[0].SunDirection.xyz);
	
	payload.Colour = Sky::Atmosphere(WorldRayOrigin(), WorldRayDirection(), Sun, GPUScene::GPUSceneScene[0].Sky);
	payload.HitDistance = -1.0;
}

#include "../RayTracingClosestHit.hlsli"
