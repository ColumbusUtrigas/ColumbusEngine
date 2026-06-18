#pragma pack_matrix(row_major)

struct RayPayload
{
	float3 Colour;
	float3 Emissive;
	float  HitDistance;
	float3 Normal;
	float3 GeometricNormal;
	uint   ObjectId;
	float2 RoughnessMetallic;
	float  Alpha;
	int    ShadingMode;
	float  AlphaCutoff;
};

#define PAYLOAD_HAS_GEOMETRIC_NORMAL 1
#define PAYLOAD_HAS_ALPHA_MASK 1

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
} Params;

[shader("raygeneration")]
void RayGen()
{
	const int2 pixel = DispatchRaysIndex().xy;
	uint RngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + (Params.RandomNumber)); // Initial seed
	const float2 PixelJitter = Random::UniformDistrubition2d(RngState);
	const float2 uv = (float2(pixel) + PixelJitter) / float2(DispatchRaysDimensions().xy);
	const float2 ndc = ScreenUVToNDC(uv);

	float4 DirectionCameraSpace = float4(ndc, DEVICE_DEPTH_FAR, 1);
	float4 DirectionWorldSpace = mul(GPUScene::GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix, DirectionCameraSpace);
	DirectionWorldSpace /= DirectionWorldSpace.w; // perspective divide
	float3 Direction = normalize(DirectionWorldSpace.xyz - GPUScene::GPUSceneScene[0].CameraCur.CameraPosition.xyz);

	float3 Sample = RayTraceAccumulateWithAlphaMask(AccelerationStructure, GPUScene::GPUSceneScene[0].CameraCur.CameraPosition.xyz,
		Direction, Params.Bounces, RngState, true);
	if (any(isnan(Sample)) || any(isinf(Sample)))
	{
		Sample = float3(0, 0, 0);
	}
	Sample = max(Sample, 0.0.xxx);

    float3 FinalColor = float3(0, 0, 0);

    if (Params.Reset == 1)
	{
        FinalColor = Sample;
    }

	if (Params.Reset == 0)
	{
		float3 previous = History[pixel].rgb;
		if (any(isnan(previous)) || any(isinf(previous)))
		{
			previous = float3(0, 0, 0);
		}
		previous = max(previous, 0.0.xxx);
		float factor = 1.0 / max(float(Params.FrameNumber), 1.0);
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
	payload.Emissive = 0.0.xxx;
	payload.HitDistance = -1.0;
}

#include "RayTracingClosestHit.hlsli"
