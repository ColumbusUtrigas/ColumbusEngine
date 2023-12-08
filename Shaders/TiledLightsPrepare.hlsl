#define LightTileSize 16
#define LightsPerTile 31
#define LightsPerGroup 64

struct LightTile
{
	uint Count;
	uint LightIds[LightsPerTile];
};

// TODO: move to a common file, it's being used in several places
struct GPULight
{
	float4 Position;
	float4 Direction;
	float4 Color;
	uint Type;
};
#define SizeofGPULight 52

#define GPULIGHT_DIRECTIONAL 0
#define GPULIGHT_POINT 1
#define GPULIGHT_SPOT 2
#define GPULIGHT_RECTANGLE 3
#define GPULIGHT_SPHERE 4

// TODO: project light sources on screen, then use atomic add on count and write light id

[[vk::binding(0, 0)]] ByteAddressBuffer LightsBuffer;
[[vk::binding(1, 0)]] RWStructuredBuffer<LightTile> TilesBuffer;

groupshared LightTile TileLDS;

[numthreads(1, 1, LightsPerGroup)]
void main(uint3 dtid : SV_DispatchThreadID, uint3 gid : SV_GroupID)
{
	uint2 TileId = dtid.xy;
	// uint LinearTileId = TileId.x
	uint LightId = dtid.z;

	// initialise LDS
	TileLDS.Count = 0;
	for (int i = 0; i < LightsPerTile; i++) TileLDS.LightIds[i] = -1;
	GroupMemoryBarrierWithGroupSync();

	// TODO: implement

	// uint tmpLightsCount = 1;

	// if (LightId >= tmpLightsCount)
	// 	return;

	// GPULight Light = LightsBuffer.Load<GPULight>(4 + LightId * SizeofGPULight);

	// switch (Light.Type)
	// {
	// 	case GPULIGHT_DIRECTIONAL:

	// }
}