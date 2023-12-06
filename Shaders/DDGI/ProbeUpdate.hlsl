struct Payload
{
	float3 Radiance;
	float HitT;
};

#ifdef RAYGEN_SHADER
#include "Defines.h"
#include "SDK/ddgi/include/DDGIRootConstants.hlsl"
#include "SDK/ddgi/Irradiance.hlsl"

struct GPULight
{
	float4 Position;
	float4 Direction;
	float4 Color;
	uint Type;
};

#define GPULIGHT_DIRECTIONAL 0
#define GPULIGHT_POINT 1
#define GPULIGHT_SPOT 2
#define GPULIGHT_RECTANGLE 3
#define GPULIGHT_SPHERE 4

// struct GPULightsBuffer
// {
// 	uint Count;
// 	GPULight Lights[];
// };

[[vk::binding(0, 0)]] StructuredBuffer<float> VerticesBuffers[1000];
[[vk::binding(0, 1)]] StructuredBuffer<uint> IndicesBuffers[1000];
[[vk::binding(0, 2)]] StructuredBuffer<float2> UvsBuffers[1000];
[[vk::binding(0, 3)]] StructuredBuffer<float> NormalsBuffers[1000];
[[vk::binding(0, 4)]] Texture2D<float4> Textures[1000];
[[vk::binding(0, 5)]] StructuredBuffer<int> MaterialsBuffers[1000];
[[vk::binding(0, 6)]] StructuredBuffer<GPULight> GPUSceneLights;

[[vk::binding(0, 7)]]
RaytracingAccelerationStructure TLAS;

[shader("raygeneration")]
void RGS()
{

}
#endif

#ifdef MISS_SHADER
[shader("miss")]
void Miss(inout Payload payload)
{

}
#endif

#ifdef CLOSEST_HIT_SHADER
[shader("closesthit")]
void ClosestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attrib)
{

}
#endif