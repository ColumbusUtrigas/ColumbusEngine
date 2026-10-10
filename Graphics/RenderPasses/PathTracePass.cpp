#include "Core/Core.h"
#include "Core/CVar.h"
#include "Graphics/Core/Pipelines.h"
#include "Graphics/Core/Texture.h"
#include "Graphics/Core/Types.h"
#include "Graphics/GPUScene.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/ShaderCache.h"
#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"

namespace Columbus
{

	ConsoleVariable<int> CVar_Bounces("r.PathTracing.Bounces", "Number of bounces for each path, default - 4", 4);

	struct PathTraceParameters
	{
		int randomNumber;
		int frameNumber;
		int reset;
		int bounces;
	};

	struct PathTraceDisplayParameters
	{
		uint FrameNumber;
	};

	struct PathTraceShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/PathTrace.csd";

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
			Desc.Name = "PathTrace";
			Desc.MaxRecursionDepth = Permutation.MaxRecursionDepth;
			return Desc;
		}

		struct Parameters
		{
			ShaderGPUScene Scene;
			ShaderAccelerationStructure AccelerationStructure;
			ShaderStorageTexture Output;
			ShaderStorageTexture History;
			ShaderPushConstants<PathTraceParameters> Constants { {}, ShaderType::Raygen };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.AccelerationStructure, 2, 0);
			Binder.Bind(Params.Output, 2, 1);
			Binder.Bind(Params.History, 2, 2);
			Binder.Bind(Params.Constants);
		}
	};

	// TODO: find a way to do it without this pass
	static void TransitionImagePass(RenderGraph& Graph, RenderGraphTextureRef Texture)
	{
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(Texture, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		Graph.AddPass("TransitionImage", RenderGraphPassType::Compute, Parameters, Dependencies, [Texture](RenderGraphContext& Context)
		{
			Context.CommandBuffer->TransitionImageLayout(Context.GetRenderGraphTexture(Texture).get(), VK_IMAGE_LAYOUT_GENERAL);
		});
	}

	PathTracingRenderContext::~PathTracingRenderContext()
	{
		Device->DestroyTexture(History);
	}

	RenderGraphTextureRef RenderPathTraced(RenderGraph& Graph, const RenderView& View, PathTracingRenderContext& PathContext)
	{
		TextureDesc2 PTTextureDesc {
			.Usage = TextureUsage::StorageSampled,
			.Width = (uint32)View.OutputSize.X, .Height = (uint32)View.OutputSize.Y,
			.Format = TextureFormat::RGBA32F,
		};
		RenderGraphTextureRef RTImage = Graph.CreateTexture(PTTextureDesc, "PathTraceTexture");

		Texture2*& HistoryTexture = PathContext.History;
		if (Graph.CreateHistoryTexture(&HistoryTexture, PTTextureDesc, "PathTraceTexture History"))
			PathContext.Frame = 0;

		RenderPassParameters Parameters;

		PathTraceShader::Parameters TraceParams;
		TraceParams.Scene.UseCombinedSampler = false;
		TraceParams.Output = RTImage;
		TraceParams.History = HistoryTexture;

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<PathTraceShader>(TraceParams);

		Graph.AddPass("PathTrace", RenderGraphPassType::Compute, Parameters, Dependencies, [TraceParams, View, &PathContext](RenderGraphContext& Context)
		{
			GPUCamera UpdatedCamera = GPUCamera(View.CameraCur);
			const bool CameraChanged = !PathContext.HasPreviousCamera || GPUCamera(PathContext.PreviousCamera) != UpdatedCamera;
			const bool reset = Context.Scene->Dirty || CameraChanged || PathContext.Frame == 0;
			if (reset) PathContext.Frame = 0;
			PathContext.PreviousCamera = View.CameraCur;
			PathContext.HasPreviousCamera = true;
			Context.Scene->MainCamera = UpdatedCamera;
			Context.Scene->Dirty = false;

			PathTraceShader::Parameters Parameters = TraceParams;
			Parameters.AccelerationStructure = Context.Scene->TLAS;
			Parameters.Constants.Value.randomNumber = (int)rand() % 20000;
			Parameters.Constants.Value.frameNumber = (int)++PathContext.Frame;
			Parameters.Constants.Value.reset = (int)reset;
			Parameters.Constants.Value.bounces = CVar_Bounces.GetValue();

			RayTracingPipeline* Pipeline = GetRayTracingPipeline<PathTraceShader>(Context, PathTraceShader::Permutation {}, PathTraceShader::PipelinePermutation {});
			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.BindRayTracingParameters<PathTraceShader>(Pipeline, Parameters);
			Context.CommandBuffer->TraceRays(Pipeline, View.OutputSize.X, View.OutputSize.Y, 1);
		});
		Graph.ExtractTexture(RTImage, &HistoryTexture);

		RenderGraphTextureRef TonemappedImage = TonemapPass(Graph, View, RTImage);
		TransitionImagePass(Graph, TonemappedImage);
		
		return TonemappedImage;
	}

}
