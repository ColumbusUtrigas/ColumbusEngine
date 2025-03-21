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

	float3 LightDirection = normalize(Light.Direction.xyz);
	float MaxDistance = 5000;
	float LightRadius = Light.SourceRadius;

	uint RngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + (Params.Random)); // Initial seed
	float3 RayDirection = float3(0,0,0);

	switch (Light.Type)
	{
	case GPULIGHT_DIRECTIONAL:
		RayDirection = Random::RandomDirectionCone(LightDirection, LightRadius, Random::UniformDistrubition2d(RngState));
		break;
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

		RayDirection = Random::RandomDirectionCone(LightDirection, LightRadius, Random::UniformDistrubition2d(RngState));

		break;
	case GPULIGHT_RECTANGLE:
	{
		LightDirection = normalize(Light.Position.xyz - origin);
		MaxDistance = distance(Light.Position.xyz, origin);

		bool twoSided = (Light.Flags & GPULIGHT_FLAG_TWOSIDED) != 0;
		// TODO: single-sided rect light early reject
		// TODO: rect light distance early reject

		// TODO: make it more stable
		float3x3 LTC_Axis = ComputeTangentsFromVector(Light.Direction.xyz);

		float2 halfSize = Light.SizeOrSpotAngles;
		// rect light tangent vectors adjusted by size
		float3 ex = LTC_Axis[0] * halfSize.x;
		float3 ey = LTC_Axis[1] * halfSize.y;

		float2 rndSample =  (Random::UniformDistrubition2d(RngState) - 0.5f) * 2; // -1..1 xy range
		float3 samplePoint = Light.Position.xyz + rndSample.x * ex + rndSample.y + ey;

		RayDirection = normalize(samplePoint - origin);
	}
		break;
	default: break;
	}

	Payload payload;
	
	// ray trace
	{
		RayDesc Ray;
		Ray.Origin = origin;
		Ray.TMin = 0.01;
		Ray.Direction = normalize(RayDirection);
		Ray.TMax = MaxDistance;
		
		TraceRay(AccelerationStructure, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
			~0, 0, 0, 0, Ray, payload);
	}

	// write results
	{
		float Result = payload.HitT > 0 ? 0.0 : 1.0;
		OutputShadow[pixel] = Result;
	}
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