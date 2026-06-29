#include "RenderPasses.h"
#include "Graphics/ShaderCache.h"

namespace Columbus
{

DECLARE_GPU_PROFILING_COUNTER(GpuCounterSky);
IMPLEMENT_GPU_PROFILING_COUNTER("Sky", "RenderGraphGPU", GpuCounterSky);

struct SkyLutShader
{
	static constexpr const char* Path = "./PrecompiledShaders/SkyLut.csd";

	struct Permutation
	{
	};

	static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
	{
	}

	struct Parameters
	{
		ShaderWriteBuffer SceneBuffer;
	};

	static void Bind(ShaderBinder& Binder, const Parameters& Params)
	{
		Binder.Bind(Params.SceneBuffer, 0, 0);
	}
};

struct SkyShader
{
	static constexpr const char* Path = "./PrecompiledShaders/Sky.csd";

	struct Permutation
	{
	};

	static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
	{
	}

	struct PipelinePermutation
	{
	};

	static GraphicsPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
	{
		GraphicsPipelineDesc Desc;
		Desc.Name = "Sky";
		Desc.rasterizerState.Cull = CullMode::No;
		Desc.blendState.RenderTargets = {
			RenderTargetBlendDesc(),
		};
		Desc.depthStencilState.DepthEnable = true;
		Desc.depthStencilState.DepthWriteMask = false;
		Desc.depthStencilState.DepthFunc = ComparisonFunc::GEqual;
		return Desc;
	}

	struct Parameters
	{
		ShaderReadBuffer SceneBuffer;
	};

	static void Bind(ShaderBinder& Binder, const Parameters& Params)
	{
		Binder.Bind(Params.SceneBuffer, 0, 0);
	}
};

void RenderPrepareSkyLut(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, DeferredRenderContext& Context)
{
	// essentially a preparation step that just creates a spherical harmonic of the sky for diffuse lookups

	RenderPassParameters Parameters;
	RenderPassDependencies Dependencies(Graph.Allocator);

	SkyLutShader::Parameters SkyParams;
	SkyParams.SceneBuffer = Graph.Scene->SceneBuffer;
	Dependencies.Bind<SkyLutShader>(SkyParams);

	Graph.AddPass("SkyLut", RenderGraphPassType::Compute, Parameters, Dependencies, [SkyParams](RenderGraphContext& Context)
	{
		RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterSky, Context);

		ComputePipeline* Pipeline = GetComputePipeline<SkyLutShader>(Context, SkyLutShader::Permutation {});
		Context.CommandBuffer->BindComputePipeline(Pipeline);
		Context.BindComputeParameters<SkyLutShader>(Pipeline, SkyParams);
		Context.CommandBuffer->Dispatch(1, 1, 1);
	});
}

void RenderDeferredSky(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext, RenderGraphTextureRef OverTexture)
{
	RenderPassParameters Parameters;
	Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, OverTexture };
	Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferDS, AttachmentClearValue{ {}, 0.0f, 0 } };
	Parameters.ViewportSize = View.RenderSize;

	RenderPassDependencies Dependencies(Graph.Allocator);

	SkyShader::Parameters SkyParams;
	SkyParams.SceneBuffer = Graph.Scene->SceneBuffer;
	Dependencies.Bind<SkyShader>(SkyParams);

	Graph.AddPass("Sky", RenderGraphPassType::Raster, Parameters, Dependencies, [SkyParams](RenderGraphContext& Context)
	{
		RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterSky, Context);

		GraphicsPipeline* Pipeline = GetGraphicsPipeline<SkyShader>(Context, SkyShader::Permutation {}, SkyShader::PipelinePermutation {});
		Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
		Context.BindGraphicsParameters<SkyShader>(Pipeline, SkyParams);
		Context.CommandBuffer->Draw(3, 1, 0, 0);
	});
}

}
