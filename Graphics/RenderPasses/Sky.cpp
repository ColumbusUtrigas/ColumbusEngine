#include "RenderPasses.h"

namespace Columbus
{

struct SkyPassParameters
{
	Matrix  InverseViewProjection;
	Vector4 CameraPosition;
	Vector4 SunDirection;
};

void RenderPrepareSkyLut(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, DeferredRenderContext& Context)
{
	// essentially a preparation step that just creates a spherical harmonic of the sky for diffuse lookups

	RenderPassParameters Parameters;
	RenderPassDependencies Dependencies(Graph.Allocator);

	Graph.AddPass("SkyLut", RenderGraphPassType::Compute, Parameters, Dependencies, [](RenderGraphContext& Context)
	{
		static ComputePipeline* Pipeline = nullptr;
		if (Pipeline == nullptr)
		{
			ComputePipelineDesc Desc;
			Desc.Name = "TAA";
			Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/SkyLut.csd");
			Pipeline = Context.Device->CreateComputePipeline(Desc);
		}

		auto Set = Context.GetDescriptorSet(Pipeline, 0);
		Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.Scene->SceneBuffer);

		Context.CommandBuffer->BindComputePipeline(Pipeline);
		Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
		Context.CommandBuffer->Dispatch(1, 1, 1);
	});
}

void RenderDeferredSky(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext, RenderGraphTextureRef OverTexture)
{
	RenderPassParameters Parameters;
	Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, OverTexture };
	Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferDS, AttachmentClearValue{ {}, 1.0f, 0 } };
	Parameters.ViewportSize = View.RenderSize;

	RenderPassDependencies Dependencies(Graph.Allocator);

	Graph.AddPass("Sky", RenderGraphPassType::Raster, Parameters, Dependencies, [View, OverTexture, Textures, &DeferredContext](RenderGraphContext& Context)
	{
		static GraphicsPipeline* Pipeline = nullptr;
		if (Pipeline == nullptr)
		{
			GraphicsPipelineDesc Desc;
			Desc.Name = "Sky";
			Desc.rasterizerState.Cull = CullMode::No;
			Desc.blendState.RenderTargets = {
				RenderTargetBlendDesc(),
			};

			Desc.depthStencilState.DepthEnable = true;
			Desc.depthStencilState.DepthWriteMask = false;
			Desc.depthStencilState.DepthFunc = ComparisonFunc::LEqual;
			Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/Sky.csd");

			Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
		}

		SkyPassParameters Parameters{
			.InverseViewProjection = View.CameraCur.GetViewProjection().GetInverted(),
			.CameraPosition = Vector4(View.CameraCur.Pos, 0),
			.SunDirection = Context.Scene->SunDirection
		};

		Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
		Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Pixel, 0, sizeof(Parameters), &Parameters);
		Context.CommandBuffer->Draw(3, 1, 0, 0);
	});
}

}
