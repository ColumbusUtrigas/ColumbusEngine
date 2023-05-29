#include "Core/Core.h"
#include "Graphics/Camera.h"
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
		Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);	// TODO: create through context
	}

	void ForwardShadingPass::PreExecute(RenderGraphContext& Context)
	{

	}

	void ForwardShadingPass::Execute(RenderGraphContext& Context)
	{
		Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
		Context.BindGPUScene(Pipeline);

		struct
		{
			Matrix M,V,P;
			uint32_t ObjectId;
		} Parameters;

		for (int i = 0; i < Context.Scene->Meshes.size(); i++)
		{
			GPUSceneMesh& Mesh = Context.Scene->Meshes[i];
			Parameters.M = Matrix(1);
			Parameters.V = MainCamera.GetViewMatrix();
			Parameters.P = MainCamera.GetProjectionMatrix();
			Parameters.ObjectId = i;

			Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex, 0, sizeof(Parameters), &Parameters);
			Context.CommandBuffer->Draw(Mesh.IndicesCount, 1, 0, 0);
		}
	}

}
