#include "Common/Image/Image.h"
#include "RenderPasses.h"

namespace Columbus
{

	void RayTracedShadowsPass::Setup(RenderGraphContext& Context)
	{
		auto ShaderSource = LoadShaderFile("RayTracedShadowsPass.glsl");

		RayTracingPipelineDesc Desc{};
		Desc.Name = "RayTracedShadows Shader";
		Desc.RayGen = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Raygen, ShaderLanguage::GLSL);
		Desc.Miss = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Miss, ShaderLanguage::GLSL);
		Desc.ClosestHit = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::ClosestHit, ShaderLanguage::GLSL);
		Desc.MaxRecursionDepth = 1;
		Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
	}

	void RayTracedShadowsPass::PreExecute(RenderGraphContext& Context)
	{
		InputNormal = Context.GetInputTexture(GBufferPass::RTNormal);
		InputDepth = Context.GetInputTexture(GBufferPass::RTDepth);
		InputWorldPosition = Context.GetInputTexture(GBufferPass::RTWorldPosition);
	}

	void RayTracedShadowsPass::Execute(RenderGraphContext& Context)
	{
		TextureDesc2 ShadowsBufferDesc;
		ShadowsBufferDesc.Width = 1280; // TODO
		ShadowsBufferDesc.Height = 720; // TODO
		ShadowsBufferDesc.Format = TextureFormat::RGBA16F;
		ShadowsBufferDesc.Usage = TextureUsage::Storage;
		auto ShadowsBuffer = Context.GetRenderTarget(RTShadows, ShadowsBufferDesc);

		auto ShadowsBufferSet = Context.GetDescriptorSet(Pipeline, 7);
		Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 0, 0, Context.Scene->TLAS);
		Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 1, 0, ShadowsBuffer);
		Context.Device->UpdateDescriptorSet(ShadowsBufferSet,2, 0, InputNormal);
		Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 3, 0, InputWorldPosition);

		Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
		Context.BindGPUScene(Pipeline);
		Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 7, 1, &ShadowsBufferSet);

		Context.CommandBuffer->TraceRays(Pipeline, 1280, 720, 1);
	}

}
