#include "Common/Image/Image.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"

namespace Columbus
{

	RenderGraphTextureRef RayTracedShadowsPass(RenderGraph& Graph, const SceneTextures& Textures, const iVector2& WindowSize)
	{
		TextureDesc2 Desc {
			.Usage = TextureUsage::Storage,
			.Width = (u32)WindowSize.X,
			.Height = (u32)WindowSize.Y,
			.Format = TextureFormat::R8,
		};
		RenderGraphTextureRef RTShadow = Graph.CreateTexture(Desc, "RayTracedShadow");

		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies;
		Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		Dependencies.Write(RTShadow, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

		Graph.AddPass("RayTraceShadow", RenderGraphPassType::Compute, Parameters, Dependencies, [RTShadow, Textures, WindowSize](RenderGraphContext& Context)
		{
			static RayTracingPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				RayTracingPipelineDesc Desc;
				Desc.Name = "RayTracedShadowsPass";
				Desc.MaxRecursionDepth = 1;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedShadowsPass.csd");

				Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
			}

			auto ShadowsBufferSet = Context.GetDescriptorSet(Pipeline, 7);
			Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 0, 0, Context.Scene->TLAS);
			Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 1, 0, Context.GetRenderGraphTexture(RTShadow).get());
			Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get());
			Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 3, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get());

			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.BindGPUScene(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 7, 1, &ShadowsBufferSet);

			Context.CommandBuffer->TraceRays(Pipeline, WindowSize.X, WindowSize.Y, 1);
		});

		return RTShadow;
	}

}
