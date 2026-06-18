#include "../Common.hlsli"

#define GPU_SCENE_NO_BINDINGS
#include "../GPUScene.hlsli"

struct VS_TO_PS
{
	float4 Pos : SV_POSITION;
};

static const float2 pos[3] = {
    float2(-1, -1),
    float2(-1, 3),
    float2(3, -1)
};

VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
	VS_TO_PS Out;
	Out.Pos = float4(pos[VertexID], 0, 1);
	return Out;
}

#define RADIANCE_CACHE_BINDING 0
#define RADIANCE_CACHE_SET 0
#include "RadianceCache.hlsli"

[[vk::binding(1, 0)]] Texture2D<float> GBufferDepth;
[[vk::binding(2, 0)]] StructuredBuffer<GPUSceneStruct> GPUSceneScene;

[[vk::push_constant]]
struct _Params {
	float3 CameraPosition;
} Params;

float3 TonemapReinhard(float3 x)
{
    return x / (x + 1);
}

float4 Pixel(VS_TO_PS In, float4 Coord : SV_Position) : SV_TARGET
{
	const uint2 Pixel = uint2(Coord.xy);
	const float Depth = GBufferDepth[Pixel];
	if (IsSkyDepth(Depth))
		return float4(0, 0, 0, 1);

    const float3 Position = ReconstructWorldPositionFromDepth(Pixel, Depth, GPUSceneScene[0].RenderSize, GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix);
    return float4(TonemapReinhard(SampleRadianceCache(Params.CameraPosition, Position)), 1);
}
