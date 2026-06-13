struct Payload
{
	float HitDistance;
	float Alpha;
	int ShadingMode;
	float AlphaCutoff;
};

#define PAYLOAD_TYPE Payload
#define PAYLOAD_HAS_ALPHA_MASK 1

#include "../GPUScene.hlsli"

#ifdef RAYGEN_SHADER
#include "../Common.hlsli"
#include "../RayTracingLightSampling.hlsli"
#include "../RayTracingAlphaMask.hlsli"

#define SET 2

// INPUTS
[[vk::binding(0, SET)]] RaytracingAccelerationStructure AccelerationStructure;
[[vk::binding(1, SET)]] Texture2D<float3> InputNormals;
[[vk::binding(2, SET)]] Texture2D<float3> InputWorldPosition;
[[vk::binding(3, SET)]] Texture2D<float>  InputDepth;

// OUTPUTS
[[vk::binding(4, SET)]] [[vk::image_format("r8")]] RWTexture2D<float> OutputShadow;

[[vk::push_constant]]
struct _Params
{
	uint Random;
	uint LightId;
} Params;

[shader("raygeneration")]
void RayGen()
{
	const int2 pixel = DispatchRaysIndex().xy;

	float depth = InputDepth[pixel].x;
	// do not trace from sky, early exit
	if (abs(depth) < EPSILON || abs(depth - 1) < EPSILON)
	{
		OutputShadow[pixel] = 0.0;
		return;
	}

	GPULight Light = GPUScene::GPUSceneLights[Params.LightId];

	// if light doesn't need shadows, early exit
	if ((Light.Flags & GPULIGHT_FLAG_SHADOW) == 0)
	{
		OutputShadow[pixel] = 1.0;
		return;
	}

	float3 origin = InputWorldPosition[pixel];

	uint RngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + (Params.Random)); // Initial seed
	RayTracedLightSample LightSample = SampleRayTracedLight(Light, origin, Random::UniformDistrubition2d(RngState));
	if (!LightSample.Valid)
	{
		OutputShadow[pixel] = 0.0;
		return;
	}

	float3 dir = normalize(LightSample.Direction);
	OutputShadow[pixel] = TraceShadowRayWithAlphaMask(AccelerationStructure, origin + dir * 0.001f, dir, LightSample.Distance, RAY_FLAG_FORCE_OPAQUE, true);
}
#endif

[shader("miss")]
void Miss(inout Payload payload)
{
	payload.HitDistance = -1.0;
}

[shader("closesthit")]
void ClosestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attrib)
{
	payload.HitDistance = RayTCurrent();
	payload.Alpha = 1.0f;
	payload.ShadingMode = MATERIAL_SHADING_OPAQUE;
	payload.AlphaCutoff = 0.5f;

	float3 barycentrics = float3(1.0f - attrib.barycentrics.x - attrib.barycentrics.y, attrib.barycentrics.x, attrib.barycentrics.y);
	GPUScene::SampleAlphaMaskHit(InstanceID(), PrimitiveIndex(), barycentrics, payload.Alpha, payload.ShadingMode, payload.AlphaCutoff);
}
