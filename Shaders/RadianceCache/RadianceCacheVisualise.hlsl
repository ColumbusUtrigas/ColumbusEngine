#include "../Common.hlsli"
#define GPU_SCENE_NO_BINDINGS
#include "../GPUScene.hlsli"
#define RADIANCE_CACHE_SET 1
#define SHARC_UPDATE 0
#define SHARC_QUERY 1
#include "RadianceCache.hlsli"

[[vk::binding(0, 0)]] StructuredBuffer<GPUSceneStruct> GPUSceneScene;
[[vk::binding(1, 0)]] Texture2D<float> GBufferDepth;
[[vk::binding(2, 0)]] Texture2D<float2> GBufferNormal;

struct VS_TO_PS
{
	float4 Position : SV_POSITION;
};

VS_TO_PS Vertex(uint VertexId : SV_VertexID)
{
	const float2 Positions[3] = { float2(-1, -1), float2(-1, 3), float2(3, -1) };
	VS_TO_PS Out;
	Out.Position = float4(Positions[VertexId], 0, 1);
	return Out;
}

float4 Pixel(VS_TO_PS In, float4 Coord : SV_Position) : SV_TARGET
{
	const uint2 Pixel = uint2(Coord.xy);
	const float Depth = GBufferDepth[Pixel];

	if (IsSkyDepth(Depth))
	{
		return float4(0, 0, 0, 1);
	}

	const float3 Position = ReconstructWorldPositionFromDepth(Pixel, Depth, GPUSceneScene[0].RenderSize, GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix);
	const float3 Normal   = NormalDecode(GBufferNormal[Pixel]);
	SharcParameters Cache = GetRadianceCacheParameters();

	if (RadianceCacheConfig.Flags.w == 1)
	{
		return float4(HashGridDebugColoredHash(Position, Normal, Cache.hashGridParameters), 1);
	}

	SharcHitData Hit = MakeRadianceCacheHit(Position, Normal);
	float3 Radiance;
	const bool Found = SharcGetCachedRadiance(Cache, Hit, Radiance, false);

	if (!Found)
	{
		return float4(0.4f, 0, 0.4f, 1);
	}

	if (RadianceCacheConfig.Flags.w == 2)
	{
		HashGridIndex Index;
		HashGridKey Key;
		Index = HashGridFindEntry(Cache.hashGridData, Position, Normal, Cache.hashGridParameters, Key);
		const float Samples = SharcGetVoxelData(Cache.resolvedBuffer, Index).accumulatedSampleNum;
		return float4(saturate(Samples / 32.0f), saturate(Samples / 8.0f), 0, 1);
	}

	Radiance = max(Radiance, 0);
	return float4(Radiance / (Radiance + 1), 1);
}
