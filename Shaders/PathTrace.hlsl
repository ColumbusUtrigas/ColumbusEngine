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

// TODO: remove ifdef
#ifdef RAYGEN_SHADER

#include "CommonRayTracing.hlsli"

#define SET 2

[[vk::binding(0, SET)]] RaytracingAccelerationStructure AccelerationStructure;
[[vk::binding(1, SET)]] RWTexture2D<float4> Output;
[[vk::binding(2, SET)]] RWTexture2D<float4> History;

// TODO: make it a cbuffer
[[vk::push_constant]]
struct _Params
{
	int RandomNumber;
	int FrameNumber;
	int Reset;
	int Bounces;
	int HasHistory;
} Params;

[shader("raygeneration")]
void RayGen()
{
	const int2 pixel = DispatchRaysIndex().xy;
	const float2 ndc = (float2(pixel) / float2(DispatchRaysDimensions().xy) * 2 - 1) * float2(1, -1);

	uint RngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + (Params.RandomNumber)); // Initial seed

	float4 DirectionCameraSpace = float4(ndc, -1, 1);
	float4 DirectionWorldSpace = mul(DirectionCameraSpace, GPUScene::GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix);
	DirectionWorldSpace /= DirectionWorldSpace.w; // perspective divide
	float3 Direction = normalize(DirectionWorldSpace.xyz - GPUScene::GPUSceneScene[0].CameraCur.CameraPosition.xyz);

	float3 Sample = RayTraceAccumulate(AccelerationStructure, GPUScene::GPUSceneScene[0].CameraCur.CameraPosition.xyz,
		Direction, Params.Bounces, RngState);

    float3 FinalColor = float3(0, 0, 0);

    if (Params.Reset == 1)
	{
        FinalColor = Sample;
    }

    if (Params.Reset == 0)
	{
		float3 previous = History[pixel].rgb;

        float sampleGray   = dot(Sample,   float3(0.2126, 0.7152, 0.0722));
        float previousGray = dot(previous, float3(0.2126, 0.7152, 0.0722));

		float factor = 1.0 / float(Params.FrameNumber);
		factor *= max(1, sampleGray - previousGray + 1);
        FinalColor = lerp(previous, Sample, factor);
    }

    Output[pixel] = float4(FinalColor, 1);
}

#endif

[shader("miss")]
void Miss(inout RayPayload payload)
{
    float3 Sun = normalize(GPUScene::GPUSceneScene[0].SunDirection.xyz);
	
    payload.Colour = Sky::Atmosphere(WorldRayOrigin(), WorldRayDirection(), Sun, GPUScene::GPUSceneScene[0].Sky);
	payload.HitDistance = -1.0;
}

#include "RayTracingClosestHit.hlsli"
