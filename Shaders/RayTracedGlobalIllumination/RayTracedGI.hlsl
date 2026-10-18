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
#include "Lighting.hlsli"

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
	float MaxRayIntensity;
} Params;

[[vk::binding(0, SET)]] RaytracingAccelerationStructure                     AccelerationStructure;
[[vk::binding(1, SET)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> Output;
[[vk::binding(2, SET)]] Texture2D<float4>                                      SampleNormals;
[[vk::binding(3, SET)]] Texture2D<float>                                       GBufferDepth;
[[vk::binding(4, SET)]] Texture2D<float4>                                      GBufferSurfaceData;

[[vk::binding(5, SET)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> OutputDirection;

// Digitally shifted 2D Sobol samples cover the hemisphere over time
// without repeatedly drawing independent directions from the same small region.
float2 SampleGIDirection(uint2 Pixel, uint Frame)
{
    const uint Seed = Random::Hash(Pixel.x ^ Random::Hash(Pixel.y));
    uint2 Sample = uint2(reversebits(Frame) ^ Seed, Random::Hash(Seed));
    uint Direction = 0x80000000u;

    for (uint Bits = Frame; Bits != 0; Bits >>= 1)
    {
        if ((Bits & 1u) != 0)
        {
            Sample.y ^= Direction;
        }

        Direction ^= Direction >> 1;
    }

    return clamp(float2(Sample) * 2.3283064365386963e-10, 1e-5, 1 - 1e-5);
}

[shader("raygeneration")]
void RayGen()
{
	const uint2 pixel = DispatchRaysIndex().xy;

	float depth = GBufferDepth[pixel].x;
	// do not trace from sky, early exit
	if (IsSkyDepth(depth))
	{
		Output[pixel]          = float4(0,0,0,0);
		OutputDirection[pixel] = 0;
		return;
	}

	const float4 Surface = GBufferSurfaceData[pixel];
    const uint PackedPixel = asuint(Surface.w) - 0x00800000u;
    const uint2 sourcePixel = uint2(PackedPixel & 0x7fffu, PackedPixel >> 15);
    const float3 WP = Surface.xyz;
    const float3 Normal = normalize(SampleNormals[pixel].xyz);

	uint RngState = Random::Hash(uint(sourcePixel.x + sourcePixel.y * GPUScene::GPUSceneScene[0].RenderSize.x) + Random::Hash(Params.Random)); // Initial seed

	const float3 Direction = RTGISampleHemisphere(Normal, SampleGIDirection(sourcePixel, Params.Random));
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

    Radiance = SanitizeRadiance(Radiance * Params.DiffuseBoost);
    const float Peak = max(Radiance.r, max(Radiance.g, Radiance.b));

    if (Params.MaxRayIntensity > 0 && Peak > Params.MaxRayIntensity)
    {
        Radiance *= Params.MaxRayIntensity / Peak;
    }

    // Keep raw incoming radiance and its world direction until probe projection.
    // Bound SH projection as well as the raw FP16 output when the limit is off.
    Radiance = min(Radiance, 8192.0);
    Output[pixel]          = float4(Radiance, payload.HitDistance);
    OutputDirection[pixel] = float4(Direction, 0);
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
