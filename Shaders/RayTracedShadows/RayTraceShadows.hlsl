struct Payload
{
	float HitT;
};

#ifdef RAYGEN_SHADER
#include "../GPUScene.hlsli"
#include "../Common.hlsli"

#define SET 2

// INPUTS
[[vk::binding(0, SET)]] RaytracingAccelerationStructure AccelerationStructure;
[[vk::binding(1, SET)]] Texture2D<float3> InputNormals;
[[vk::binding(2, SET)]] Texture2D<float3> InputWorldPosition;
[[vk::binding(3, SET)]] Texture2D<float>  InputDepth;

// OUTPUTS
[[vk::binding(4, SET)]] RWTexture2D<float> OutputShadow;

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
	// do not trace from sky
	if (abs(depth) < EPSILON || abs(depth - 1) < EPSILON)
	{
		OutputShadow[pixel] = 0.0;
		return;
	}

	float3 origin = InputWorldPosition[pixel];

	GPULight Light = GPUScene::GPUSceneLights[Params.LightId];

	float3 LightDirection = normalize(Light.Direction.xyz);
	float MaxDistance = 5000;
	float LightRadius = Light.SourceRadius;

	switch (Light.Type)
	{
	case GPULIGHT_DIRECTIONAL: break;
	case GPULIGHT_POINT:
	case GPULIGHT_SPOT:
		LightDirection = normalize(Light.Position.xyz - origin);
		MaxDistance = distance(Light.Position.xyz, origin);
		LightRadius = Light.SourceRadius / MaxDistance;

		// TODO: spotlight early reject

		if (MaxDistance > Light.Range)
		{
			OutputShadow[pixel] = 0.0;
			return;
		}

		break;
	case GPULIGHT_RECTANGLE: // TODO: better sampling
		LightDirection = normalize(Light.Position.xyz - origin);
		MaxDistance = distance(Light.Position.xyz, origin);
		break;
	default: break;
	}

    uint RngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + (Params.Random)); // Initial seed

    float3 direction = Random::RandomDirectionCone(LightDirection, LightRadius, Random::UniformDistrubition2d(RngState));
	direction = normalize(direction);
	
	Payload payload;
	
    RayDesc Ray;
    Ray.Origin = origin;
    Ray.TMin = 0.01;
    Ray.Direction = direction;
    Ray.TMax = MaxDistance;
	
    TraceRay(AccelerationStructure, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
		~0, 0, 0, 0, Ray, payload);

	float Result = payload.HitT > 0 ? 0.0 : 1.0;
	OutputShadow[pixel] = Result;
}
#endif

[shader("miss")]
void Miss(inout Payload payload)
{
	payload.HitT = -1.0;
}

[shader("closesthit")]
void ClosestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attrib)
{
	payload.HitT = RayTCurrent();
}