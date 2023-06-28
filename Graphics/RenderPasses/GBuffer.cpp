#include "Core/Core.h"
#include "RenderPasses.h"

namespace Columbus
{

	struct PerObjectParameters
	{
		Matrix M,V,P;
		uint32_t ObjectId;
	};

	void GBufferPass::Setup(RenderGraphContext& Context)
	{
		auto ShaderSource = LoadShaderFile("GBufferPass.glsl");
		GraphicsPipelineDesc Desc;
		Desc.Name = "GBufferPass";
		Desc.rasterizerState.Cull = CullMode::No;
		Desc.VS = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Vertex, ShaderLanguage::GLSL);
		Desc.PS = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Pixel, ShaderLanguage::GLSL);
		Desc.blendState.RenderTargets = {
			RenderTargetBlendDesc(),
			RenderTargetBlendDesc(),
			RenderTargetBlendDesc(),
			RenderTargetBlendDesc(),
		};
		Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
	}

	void GBufferPass::PreExecute(RenderGraphContext& Context)
	{

	}

	void GBufferPass::Execute(RenderGraphContext& Context)
	{
		Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
		Context.BindGPUScene(Pipeline);

		PerObjectParameters Parameters;

		for (int i = 0; i < Context.Scene->Meshes.size(); i++)
		{
			GPUSceneMesh& Mesh = Context.Scene->Meshes[i];
			Parameters.M = Matrix(1);
			Parameters.V = MainCamera.GetViewMatrix();
			Parameters.P = MainCamera.GetProjectionMatrix();
			Parameters.ObjectId = i;

			Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(Parameters), &Parameters);
			Context.CommandBuffer->Draw(Mesh.IndicesCount, 1, 0, 0);
		}
	}

	void GBufferCompositePass::Setup(RenderGraphContext& Context)
	{
		auto ShaderSource = LoadShaderFile("GBufferCompositePass.glsl");
		GraphicsPipelineDesc Desc;
		Desc.Name = "GBufferCompositePass";
		Desc.rasterizerState.Cull = CullMode::No;
		Desc.VS = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Vertex, ShaderLanguage::GLSL);
		Desc.PS = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Pixel, ShaderLanguage::GLSL);
		Desc.blendState.RenderTargets = {
			RenderTargetBlendDesc()
		};
		Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
	}

	void GBufferCompositePass::PreExecute(RenderGraphContext& Context)
	{
		InputAlbedo = Context.GetInputTexture(GBufferPass::RTAlbedo);
		InputNormal = Context.GetInputTexture(GBufferPass::RTNormal);
		InputShadowBuffer = Context.GetInputTexture(RayTracedShadowsPass::RTShadows);
	}

	void GBufferCompositePass::Execute(RenderGraphContext& Context)
	{
		auto Set = Context.GetDescriptorSet(Pipeline, 0);
		Context.Device->UpdateDescriptorSet(Set, 0, 0, InputAlbedo);
		Context.Device->UpdateDescriptorSet(Set, 1, 0, InputNormal);
		Context.Device->UpdateDescriptorSet(Set, 2, 0, InputShadowBuffer);

		Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
		Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &Set);
		Context.CommandBuffer->Draw(3, 1, 0, 0);
	}

}
