#include "Core/Core.h"
#include "Graphics/Types.h"
#include "RenderPasses.h"

namespace Columbus
{

	void ForwardShadingPass::Setup(RenderGraphContext& Context)
	{
		auto Source = LoadShaderFile("ForwardShader.glsl");

		GraphicsPipelineDesc Desc;
		Desc.Name = "Forward Shader";
		Desc.rasterizerState.Cull = CullMode::No;
		Desc.VS = std::make_shared<ShaderStage>(Source, "main", ShaderType::Vertex, ShaderLanguage::GLSL);
		Desc.PS = std::make_shared<ShaderStage>(Source, "main", ShaderType::Pixel, ShaderLanguage::GLSL);
		Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
	}

	void ForwardShadingPass::Execute(RenderGraphContext& Context)
	{
		Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
		Context.BindGPUScene(Pipeline);

		struct
		{
			Matrix MVP;
			uint32_t ObjectId;
		} Parameters;

		for (int i = 0; i < Context.Scene->Meshes.size(); i++)
		{
			GPUSceneMesh& Mesh = Context.Scene->Meshes[i];
			Parameters.MVP = Mesh.Transform * MainCamera.GetViewProjection();
			Parameters.ObjectId = i;

			Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex, 0, sizeof(Parameters), &Parameters);
			Context.CommandBuffer->Draw(Mesh.VertexCount, 1, 0, 0);
		}
	}

}
