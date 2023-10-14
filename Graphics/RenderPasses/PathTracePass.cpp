#include "Core/Core.h"
#include "Graphics/Core/Pipelines.h"
#include "Graphics/Core/Texture.h"
#include "Graphics/Core/Types.h"
#include "Graphics/GPUScene.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	// no need to GPUParametrize push constants?
	struct PathTraceParameters
	{
		GPUCamera MainCamera;
		int frameNumber;
		int reset;
		int bounces;
	};

	struct PathTraceDisplayParameters
	{
		uint FrameNumber;
	};

	void RenderPathTraced(RenderGraph& Graph, const Camera& MainCamera, const iVector2& WindowSize)
	{
		TextureDesc2 PTTextureDesc {
			.Usage = TextureUsage::Storage,
			.Width = (uint32)WindowSize.X, .Height = (uint32)WindowSize.Y,
			.Format = TextureFormat::RGBA16F,
		};
		RenderGraphTextureRef RTImage = Graph.CreateTexture(PTTextureDesc, "PathTraceTexture");

		{
			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies;
			Dependencies.Write(RTImage, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass("PathTrace", RenderGraphPassType::Compute, Parameters, Dependencies, [RTImage, &MainCamera, WindowSize](RenderGraphContext& Context)
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

				auto RTSet = Context.GetDescriptorSet(PTPipeline, 7);
				Context.Device->UpdateDescriptorSet(RTSet, 0, 0, Context.Scene->TLAS); // TODO: move to unified scene set
				Context.Device->UpdateDescriptorSet(RTSet, 1, 0, Context.GetRenderGraphTexture(RTImage).get());

				GPUCamera UpdatedCamera = GPUCamera(MainCamera);
				Context.Scene->Dirty = Context.Scene->MainCamera != UpdatedCamera; // TODO: move to the main rendering system
				Context.Scene->MainCamera = UpdatedCamera;

				bool reset = Context.Scene->Dirty;
				reset = true;
				int frame = rand() % 2000;
				int bounces = 2;

				PathTraceParameters rayParams = { UpdatedCamera, (int)frame, (int)reset, bounces };

				Context.CommandBuffer->BindRayTracingPipeline(PTPipeline);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(PTPipeline, 7, 1, &RTSet);
				Context.BindGPUScene(PTPipeline);

				Context.CommandBuffer->PushConstantsRayTracing(PTPipeline, ShaderType::Raygen, 0, sizeof(rayParams), &rayParams);
				Context.CommandBuffer->TraceRays(PTPipeline, WindowSize.X, WindowSize.Y, 1);
			});
		}

		TonemapPass(Graph, RTImage);
	}

}
