struct RayPayload
{
	float3 Colour;
	float3 Emissive;
	float HitDistance;
	float3 Normal;
	uint ObjectId;
	float2 RoughnessMetallic;
};

#include "../GPUScene.hlsli"
#include "../RayTracingShadowVisibility.hlsli"
#include "../Common.hlsli"
#include "../SkyCommon.hlsli"

#ifdef RAYGEN_SHADER

#include "../CommonRayTracing.hlsli"
#include "ReflectionCaptureBakeCommon.hlsli"

#define SET 2

[[vk::binding(0, SET)]] RaytracingAccelerationStructure SceneTLAS;
[[vk::binding(1, SET)]] RWTexture2DArray<float4> RadianceMip0;

// mirrored in Graphics/RenderPasses/ReflectionCaptures.cpp
[[vk::push_constant]]
struct _Params
{
	float4 CapturePosition; // xyz: world position, w: unused

	int Resolution;
	int SampleOffset;
	int SamplesThisPass;
	int TotalSamples;

	int Bounces;
	int _Pad0;
	int _Pad1;
	int _Pad2;
} Params;

// dispatched as (Resolution, 6 * Resolution); y selects the cube face
[shader("raygeneration")]
void RayGen()
{
	uint Resolution = (uint)Params.Resolution;
	uint2 Pixel = DispatchRaysIndex().xy;
	uint Face = Pixel.y / Resolution;
	if (Pixel.x >= Resolution || Face >= 6u)
		return;

	uint2 FacePixel = uint2(Pixel.x, Pixel.y - Face * Resolution);
	float3 Direction = ReflectionCaptureCubeFaceDirection(Face, FacePixel, Resolution);

	uint TotalSamples = (uint)max(Params.TotalSamples, 1);
	uint SampleOffset = (uint)Params.SampleOffset;
	if (SampleOffset >= TotalSamples)
		return;

	uint SamplesThisPass = min((uint)max(Params.SamplesThisPass, 1), TotalSamples - SampleOffset);
	bool IsFirstSampleChunk = SampleOffset == 0;
	bool IsLastSampleChunk = SampleOffset + SamplesThisPass >= TotalSamples;

	// sample chunks accumulate in place; weight lives in .a until the last chunk normalizes
	uint3 Texel = uint3(FacePixel, Face);
	float4 Accumulated = IsFirstSampleChunk ? float4(0, 0, 0, 0) : RadianceMip0[Texel];
	float3 RadianceSum = Accumulated.rgb;
	float WeightSum = Accumulated.a;

	uint TexelIndex = Face * Resolution * Resolution + FacePixel.y * Resolution + FacePixel.x;

	[loop]
	for (uint LocalSample = 0; LocalSample < SamplesThisPass; LocalSample++)
	{
		uint SampleIndex = SampleOffset + LocalSample;
		uint RngState = Random::Hash(TexelIndex ^ Random::Hash(SampleIndex + 1u));
		RadianceSum += RayTraceAccumulate(SceneTLAS, Params.CapturePosition.xyz, Direction, Params.Bounces, RngState);
		WeightSum += 1.0;
	}

	if (IsLastSampleChunk)
	{
		RadianceSum /= max(WeightSum, 0.0001);
		WeightSum = 1.0;
	}

	RadianceMip0[Texel] = float4(RadianceSum, WeightSum);
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
