#include "RadianceCache.h"
#include "Graphics/ShaderCache.h"

namespace Columbus::RadianceCache
{

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterTraceRadianceCache);

	IMPLEMENT_GPU_PROFILING_COUNTER("RadianceCache", "RenderGraphGPU", GpuCounterTraceRadianceCache);

	static constexpr int NumCascades = 6;
	static constexpr int CascadeResolution = 16;

	struct RadianceCacheEntry
	{
		Vector4 Irradiance; // just plain omnidirectional irradiance for now
	};

	struct RadianceCacheTracingParams
	{
		Vector3 CameraPosition{};
		int Random = 0;
	};

	struct RadianceCacheTraceShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RadianceCache/RadianceCacheTrace.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct PipelinePermutation
		{
			u32 MaxRecursionDepth = 1;
		};

		static RayTracingPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			RayTracingPipelineDesc Desc {};
			Desc.Name = "RadianceCacheTrace";
			Desc.MaxRecursionDepth = Permutation.MaxRecursionDepth;
			return Desc;
		}

		struct Parameters
		{
			ShaderGPUScene Scene;
			ShaderAccelerationStructure AccelerationStructure;
			ShaderWriteBuffer RadianceCache;
			ShaderPushConstants<RadianceCacheTracingParams> Constants { {}, ShaderType::Raygen };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.AccelerationStructure, 2, 0);
			Binder.Bind(Params.RadianceCache, 2, 1);
			Binder.Bind(Params.Constants);
		}
	};

	void TraceRadianceCache(RenderGraph& Graph, RenderView& View, RadianceCacheData& RadianceCache)
	{
		const int Resolution = CascadeResolution * CascadeResolution * CascadeResolution;
		BufferDesc Desc(sizeof(RadianceCacheEntry) *  Resolution * NumCascades, BufferType::UAV);
		RadianceCache.DataBuffer = Graph.CreateBuffer(Desc, "RadianceCache");

		// 2. run tracing shader
		RenderPassParameters Parameters;

		RadianceCacheTraceShader::Parameters TraceParams;
		TraceParams.Scene.UseCombinedSampler = false;
		TraceParams.RadianceCache = RadianceCache.DataBuffer;
		TraceParams.Constants.Value = {
			.CameraPosition = View.CameraCur.Pos,
			.Random = 0,
		};

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RadianceCacheTraceShader>(TraceParams);

		Graph.AddPass("RadianceCacheTrace", RenderGraphPassType::Compute, Parameters, Dependencies, [TraceParams](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterTraceRadianceCache, Context);

			RadianceCacheTraceShader::Parameters Parameters = TraceParams;
			Parameters.AccelerationStructure = Context.Scene->TLAS;
			Parameters.Constants.Value.Random = rand();

			RayTracingPipeline* Pipeline = GetRayTracingPipeline<RadianceCacheTraceShader>(Context, RadianceCacheTraceShader::Permutation {}, RadianceCacheTraceShader::PipelinePermutation {});
			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.BindRayTracingParameters<RadianceCacheTraceShader>(Pipeline, Parameters);
			Context.CommandBuffer->TraceRays(Pipeline, NumCascades, CascadeResolution*CascadeResolution*CascadeResolution, 1);
		});
	}

	struct RadianceCacheVisualiseParams
	{
		Vector3 CameraPosition{};
	};

	struct RadianceCacheVisualiseShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RadianceCache/RadianceCacheVisualise.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct PipelinePermutation
		{
			CullMode Cull = CullMode::No;
		};

		static GraphicsPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			GraphicsPipelineDesc Desc;
			Desc.Name = "RadianceCacheVis";
			Desc.rasterizerState.Cull = Permutation.Cull;
			Desc.blendState.RenderTargets = { RenderTargetBlendDesc() };
			Desc.depthStencilState.DepthEnable = false;
			Desc.depthStencilState.DepthWriteMask = false;
			return Desc;
		}

		struct Parameters
		{
			ShaderReadBuffer RadianceCache;
			ShaderSampledTexture GBufferDepth { TextureBindingFlags::AspectDepth };
			ShaderReadBuffer GPUSceneScene;
			ShaderPushConstants<RadianceCacheVisualiseParams> Constants { {}, ShaderType::Pixel };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.RadianceCache, 0, 0);
			Binder.Bind(Params.GBufferDepth, 0, 1);
			Binder.Bind(Params.GPUSceneScene, 0, 2);
			Binder.Bind(Params.Constants);
		}
	};

	RenderGraphTextureRef VisualiseRadianceCache(RenderGraph& Graph, RenderView& View, RadianceCacheData& RadianceCache, RenderGraphTextureRef GBufferDepth)
	{
		iVector2 Size = View.RenderSize;

		TextureDesc2 ResultDesc;
		ResultDesc.Format = TextureFormat::R11G11B10F; // format to get less banding
		ResultDesc.Usage = TextureUsage::RenderTargetColor;
		ResultDesc.Width = (u32)Size.X;
		ResultDesc.Height = (u32)Size.Y;

		RenderGraphTextureRef Target = Graph.CreateTexture(ResultDesc, "VisualisationResult");

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Target, {} };
		Parameters.ViewportSize = Size;

		RadianceCacheVisualiseShader::Parameters VisualiseParams;
		VisualiseParams.RadianceCache = RadianceCache.DataBuffer;
		VisualiseParams.GBufferDepth = GBufferDepth;
		VisualiseParams.Constants.Value.CameraPosition = View.CameraCur.Pos;

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RadianceCacheVisualiseShader>(VisualiseParams);

		Graph.AddPass("Radiance Cache Visualisation", RenderGraphPassType::Raster, Parameters, Dependencies, [VisualiseParams](RenderGraphContext& Context)
		{
			RadianceCacheVisualiseShader::Parameters Parameters = VisualiseParams;
			Parameters.GPUSceneScene = Context.Scene->SceneBuffer;

			GraphicsPipeline* Pipeline = GetGraphicsPipeline<RadianceCacheVisualiseShader>(Context, RadianceCacheVisualiseShader::Permutation {}, RadianceCacheVisualiseShader::PipelinePermutation {});
			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.BindGraphicsParameters<RadianceCacheVisualiseShader>(Pipeline, Parameters);
			Context.CommandBuffer->Draw(3, 1, 0, 0);
		});

		return Target;
	}

}
