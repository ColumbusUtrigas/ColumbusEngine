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
	float3 Sun = normalize(float3(1, 1, 1)); // TODO: put sun direction into scene description
	
	payload.Colour = Sky::Atmosphere(WorldRayOrigin(), WorldRayDirection(), Sun);
	payload.HitDistance = -1.0;
}

float2 BaryLerp(float2 a, float2 b, float2 c, float3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

float3 BaryLerp(float3 a, float3 b, float3 c, float3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

// TODO: common closesthit
[shader("closesthit")]
void ClosestHit(inout RayPayload payload, BuiltInTriangleIntersectionAttributes attrib)
{
	float3 barycentrics = float3(1.0f - attrib.barycentrics.x - attrib.barycentrics.y, attrib.barycentrics.x, attrib.barycentrics.y);
	
	uint ObjectId = InstanceID();

	GPUSceneMeshCompact Mesh = GPUScene::GPUSceneMeshes[NonUniformResourceIndex(InstanceID())];

	GPUScene::Vertex vert1 = GPUScene::FetchVertex(NonUniformResourceIndex(ObjectId), NonUniformResourceIndex(PrimitiveIndex() * 3 + 0));
	GPUScene::Vertex vert2 = GPUScene::FetchVertex(NonUniformResourceIndex(ObjectId), NonUniformResourceIndex(PrimitiveIndex() * 3 + 1));
	GPUScene::Vertex vert3 = GPUScene::FetchVertex(NonUniformResourceIndex(ObjectId), NonUniformResourceIndex(PrimitiveIndex() * 3 + 2));

	float2 uv = BaryLerp(vert1.UV, vert2.UV, vert3.UV, barycentrics);
	float3 normal = normalize(BaryLerp(vert1.Normal, vert2.Normal, vert3.Normal, barycentrics));

	int matid = Mesh.MaterialId;

	GPUMaterialSampledData Material = GPUScene::SampleMaterial(matid, uv);

	payload.Colour = Material.Albedo;
	payload.HitDistance = RayTCurrent();
	payload.Normal = normal;
	payload.ObjectId = ObjectId;
	payload.RoughnessMetallic = float2(Material.Roughness, Material.Metallic);
}