#include "Core/Core.h"
#include "Core/CVar.h"
#include "Graphics/Core/Pipelines.h"
#include "Graphics/Core/Texture.h"
#include "Graphics/Core/Types.h"
#include "Graphics/GPUScene.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"

namespace Columbus
{

	// TODO: temporal accumulation, rays per pixel

	ConsoleVariable<int> CVar_Bounces("r.PathTracing.Bounces", "Number of bounces for each path, default - 2", 2);

	// no need to GPUParametrize push constants?
	struct PathTraceParameters
	{
		GPUCamera MainCamera;
		int randomNumber;
		int frameNumber;
		int reset;
		int bounces;
	};

	struct PathTraceDisplayParameters
	{
		uint FrameNumber;
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

	RenderGraphTextureRef RenderPathTraced(RenderGraph& Graph, const RenderView& View)
	{
		TextureDesc2 PTTextureDesc {
			.Usage = TextureUsage::Storage,
			.Width = (uint32)View.OutputSize.X, .Height = (uint32)View.OutputSize.Y,
			.Format = TextureFormat::RGBA32F,
		};
		RenderGraphTextureRef RTImage = Graph.CreateTexture(PTTextureDesc, "PathTraceTexture");

		// TODO: move to RenderView/PathTracingContext?
		static Texture2* HistoryTexture = nullptr;

		Graph.CreateHistoryTexture(&HistoryTexture, PTTextureDesc, "PathTraceTexture History");

		{
			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Write(RTImage, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass("PathTrace", RenderGraphPassType::Compute, Parameters, Dependencies, [RTImage, View](RenderGraphContext& Context)
			{
				static RayTracingPipeline* PTPipeline = nullptr;
				if (PTPipeline == nullptr)
				{
					RayTracingPipelineDesc Desc{};
					Desc.Name = "RTXON";
					Desc.MaxRecursionDepth = 1;
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/PathTrace.csd");
					PTPipeline = Context.Device->CreateRayTracingPipeline(Desc);
				}

				auto RTSet = Context.GetDescriptorSet(PTPipeline, 2);
				Context.Device->UpdateDescriptorSet(RTSet, 0, 0, Context.Scene->TLAS); // TODO: move to unified scene set
				Context.Device->UpdateDescriptorSet(RTSet, 1, 0, Context.GetRenderGraphTexture(RTImage).get());
				Context.Device->UpdateDescriptorSet(RTSet, 2, 0, HistoryTexture);

				GPUCamera UpdatedCamera = GPUCamera(View.CameraCur);
				Context.Scene->Dirty = Context.Scene->MainCamera != UpdatedCamera; // TODO: move to the main rendering system
				Context.Scene->MainCamera = UpdatedCamera;

				bool reset = Context.Scene->Dirty;

				// TODO: move to RenderView/PathTracingContext?
				static int frame = 1;

				if (reset)
					frame = 1;

				PathTraceParameters rayParams = {
					UpdatedCamera,
					(int)rand() % 20000,
					(int)frame++,
					(int)reset,
					CVar_Bounces.GetValue(),
				};

				Context.CommandBuffer->BindRayTracingPipeline(PTPipeline);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(PTPipeline, 2, 1, &RTSet);
				Context.BindGPUScene(PTPipeline);

				Context.CommandBuffer->PushConstantsRayTracing(PTPipeline, ShaderType::Raygen, 0, sizeof(rayParams), &rayParams);
				Context.CommandBuffer->TraceRays(PTPipeline, View.OutputSize.X, View.OutputSize.Y, 1);
			});
			Graph.ExtractTexture(RTImage, &HistoryTexture);
		}

		RenderGraphTextureRef TonemappedImage = TonemapPass(Graph, View, RTImage);
		//DebugUIPass(Graph, View, TonemappedImage);
		TransitionImagePass(Graph, TonemappedImage);
		
		return TonemappedImage;
	}

}
