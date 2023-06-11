#include "Core/Core.h"
#include "Graphics/Camera.h"
#include "Graphics/IrradianceVolume.h"
#include "Graphics/Types.h"
#include "RenderPasses.h"

namespace Columbus
{

	struct IrradianceParameters
	{
		Matrix View, Projection;
	};

	Buffer* IrradianceProbeBuffer; // TODO

	void ForwardShadingPass::Setup(RenderGraphContext& Context)
	{
		{
			auto Source = LoadShaderFile("ForwardShader.glsl");

			GraphicsPipelineDesc Desc;
			Desc.Name = "Forward Shader";
			Desc.rasterizerState.Cull = CullMode::No;
			Desc.VS = std::make_shared<ShaderStage>(Source, "main", ShaderType::Vertex, ShaderLanguage::GLSL);
			Desc.PS = std::make_shared<ShaderStage>(Source, "main", ShaderType::Pixel, ShaderLanguage::GLSL);
			Desc.blendState.RenderTargets = {
				RenderTargetBlendDesc()
			};
			Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);	// TODO: create through context
		}

		{
			auto Source = LoadShaderFile("ProbeVisualise.glsl");

			GraphicsPipelineDesc Desc;
			Desc.Name = "Probe Visualise Shader";
			Desc.rasterizerState.Cull = CullMode::No;
			Desc.VS = std::make_shared<ShaderStage>(Source, "main", ShaderType::Vertex, ShaderLanguage::GLSL);
			Desc.PS = std::make_shared<ShaderStage>(Source, "main", ShaderType::Pixel, ShaderLanguage::GLSL);
			Desc.blendState.RenderTargets = {
				RenderTargetBlendDesc()
			};
			ProbeVisPipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);	// TODO: create through context
		}
	}

	void ForwardShadingPass::PreExecute(RenderGraphContext& Context)
	{
		IrradianceProbeBuffer = Context.GetInputBuffer(IrradianceProbeTracePass::ProbeBufferName);
	}

	void ForwardShadingPass::Execute(RenderGraphContext& Context)
	{
		auto IrradianceProbeSet = Context.GetDescriptorSet(Pipeline, 7);
		Context.Device->UpdateDescriptorSet(IrradianceProbeSet, 0, 0, IrradianceProbeBuffer);

		Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
		Context.BindGPUScene(Pipeline);
		Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 7, 1, &IrradianceProbeSet);

		struct
		{
			IrradianceVolume Volume;

			Matrix M,V,P;
			uint32_t ObjectId;
		} Parameters;
		Parameters.Volume = Volume;

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

		IrradianceParameters IrradianceParams { MainCamera.GetViewMatrix(), MainCamera.GetProjectionMatrix() };
		auto IrradianceProbeSetVisualize = Context.GetDescriptorSet(ProbeVisPipeline, 0);
		Context.Device->UpdateDescriptorSet(IrradianceProbeSetVisualize, 0, 0, IrradianceProbeBuffer);

		Context.CommandBuffer->BindGraphicsPipeline(ProbeVisPipeline);
		Context.CommandBuffer->PushConstantsGraphics(ProbeVisPipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(IrradianceParams), &IrradianceParams);
		Context.CommandBuffer->BindDescriptorSetsGraphics(ProbeVisPipeline, 0, 1, &IrradianceProbeSetVisualize);
		Context.CommandBuffer->Draw(6*Volume.GetTotalProbes(), 1, 0, 0);
	}

}
