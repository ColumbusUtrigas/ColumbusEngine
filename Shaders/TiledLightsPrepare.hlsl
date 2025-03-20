#define LightTileSize 16
#define LightsPerTile 31
#define LightsPerGroup 64

struct LightTile
{
	uint Count;
	uint LightIds[LightsPerTile];
};

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
}