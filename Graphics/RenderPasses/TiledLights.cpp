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

		BufferDesc Desc(TilesCount * sizeof(LightTile), BufferType::UAV);
		RenderGraphBufferRef TilesBuffer = Graph.CreateBuffer(Desc, "TiledLightsBuffer");

		// TODO: run compute shader on light sources, which projects them onto screen and puts into a tile
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies;
		Dependencies.WriteBuffer(TilesBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("PrepareTiledLights", RenderGraphPassType::Compute, Parameters, Dependencies, [TilesBuffer](RenderGraphContext& Context)
		{
			static ComputePipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				ComputePipelineDesc Desc;
				Desc.Name = "TiledLightsPrepare";
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/TiledLightsPrepare.csd");
				Context.Device->CreateComputePipeline(Desc);
			}

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			// TODO: implement
		});

		// TODO: visualise tiles as a heatmap in a debug overlay pass
	}

}