#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"

namespace Columbus
{

	// must match shader definition
	static constexpr int LightTileSize = 16;
	static constexpr int LightsPerTile = 31;
	static constexpr int LightsPerGroup = 64;

	// used only to create a buffer of an appropriate size
	struct LightTile
	{
		u32 Count;
		u32 LightIds[LightsPerTile];
	};

	void PrepareTiledLights(RenderGraph& Graph, const RenderView& View)
	{
		return;
		// divide screen into tiles of some size and create RG buffer for them
		iVector2 TilesCount2d = (View.OutputSize + (LightTileSize - 1)) / LightTileSize;
		int TilesCount = TilesCount2d.X * TilesCount2d.Y;

		// TODO: implement
	}

}