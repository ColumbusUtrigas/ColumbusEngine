#pragma once

// LightTileSize and LightTileBits are mirrored in Graphics/RenderPasses/TiledLights.cpp.
static const uint LightTileSize = 16;
static const uint LightTileBits = 32;

uint2 GetLightTileCount(uint2 RenderSize)
{
	return (RenderSize + LightTileSize - 1) / LightTileSize;
}

uint GetLightTileWordCount(uint LightsCount)
{
	return max((LightsCount + LightTileBits - 1) / LightTileBits, 1u);
}
