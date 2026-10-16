#include "Core/CVar.h"
#include "Graphics/ShaderCache.h"
#include "Profiling/Profiling.h"
#include "RenderPasses.h"

#include <algorithm>

namespace Columbus
{
	ConsoleVariable<bool> CVar_TiledLights("r.TiledLights", "Cull deferred lights per screen tile; disable for an all-lights reference.", true);

	DECLARE_CPU_PROFILING_COUNTER(CpuCounterPrepareTiledLights);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterPrepareTiledLights);

	IMPLEMENT_CPU_PROFILING_COUNTER("Prepare tiled lights", "Render", CpuCounterPrepareTiledLights);
	IMPLEMENT_GPU_PROFILING_COUNTER("Prepare tiled lights", "RenderGraphGPU", GpuCounterPrepareTiledLights);

	// Mirrored in Shaders/TiledLightsCommon.hlsli. Each bit is one dense GPUScene light index.
	static constexpr u32 LightTileSize = 16;
	static constexpr u32 LightTileBits = 32;

	struct TiledLightsPrepareShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/TiledLightsPrepare.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>&)
		{
		}

		struct Parameters
		{
			ShaderSampledTexture Depth{ TextureBindingFlags::AspectDepth };
			ShaderReadBuffer     Lights;
			ShaderReadBuffer     Scene;
			ShaderWriteBuffer    Tiles;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Depth, 0, 0);
			Binder.Bind(Params.Lights, 0, 1);
			Binder.Bind(Params.Scene, 0, 2);
			Binder.Bind(Params.Tiles, 0, 3);
		}
	};

	void PrepareTiledLights(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& DeferredContext)
	{
		PROFILE_CPU(CpuCounterPrepareTiledLights);
		DeferredContext.LightTiles = -1;

		if (!CVar_TiledLights.GetValue())
		{
			return;
		}

		const iVector2 TileCount    = (View.RenderSize + (LightTileSize - 1)) / LightTileSize;
		const u32      WordsPerTile = std::max((u32(Graph.Scene->Lights.Size()) + LightTileBits - 1) / LightTileBits, 1u);
		const u64      BufferSize   = u64(TileCount.X) * u64(TileCount.Y) * WordsPerTile * sizeof(u32);

		BufferDesc Desc(BufferSize, BufferType::UAV);
		DeferredContext.LightTiles = Graph.CreateBuffer(Desc, "TiledLights");

		TiledLightsPrepareShader::Parameters ShaderParams;
		ShaderParams.Depth  = Textures.GBufferDS;
		ShaderParams.Lights = Graph.Scene->LightsBuffer;
		ShaderParams.Scene  = Graph.Scene->SceneBuffer;
		ShaderParams.Tiles  = DeferredContext.LightTiles;

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<TiledLightsPrepareShader>(ShaderParams);

		Graph.AddPass("PrepareTiledLights", RenderGraphPassType::Compute, Parameters, Dependencies,
			[TileCount, ShaderParams](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterPrepareTiledLights, Context);

				ComputePipeline* Pipeline = GetComputePipeline<TiledLightsPrepareShader>(Context, {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.BindComputeParameters<TiledLightsPrepareShader>(Pipeline, ShaderParams);
				Context.CommandBuffer->Dispatch(u32(TileCount.X), u32(TileCount.Y), 1);
			}
		);
	}
}
