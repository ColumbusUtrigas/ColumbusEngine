#include "Graphics/RenderGraph.h"

namespace Columbus
{

	struct UIPerDrawcallParams
	{
		Vector4 Colour;
		Vector2 Position;
		Vector2 Size;
	};

	void RenderUIPass(RenderGraph& Graph, RenderView& View, RenderGraphTextureRef TextureToDrawTo)
	{
		if (View.UI == nullptr)
			return;

		RenderPassParameters Parameters;
		Parameters.ViewportSize = View.OutputSize;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, TextureToDrawTo, {} };

		Graph.AddPass("UI", RenderGraphPassType::Raster, Parameters, Dependencies, [View, TextureToDrawTo](RenderGraphContext& Context)
		{
			// TODO: shader system
			static GraphicsPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				GraphicsPipelineDesc Desc;
				Desc.Name = "UI";
				Desc.rasterizerState.Cull = CullMode::No;
				Desc.blendState.RenderTargets = {
					RenderTargetBlendDesc {
						.BlendEnable = true,
						.SrcBlend = Blend::SrcAlpha,
						.DestBlend = Blend::InvSrcAlpha,
					}
				};

				Desc.depthStencilState.DepthEnable = false;
				Desc.depthStencilState.DepthWriteMask = false;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/UI.csd");

				Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);

			for (const UIImage* Img : View.UI->Images)
			{
				UIPerDrawcallParams Params;
				Params.Colour = Img->Colour;
				Params.Position = Img->Position;
				Params.Size = Img->GetActualRenderSize(View.CameraCur.GetAspect());

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Img->Img, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.Device->GetStaticSampler());

				Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(Params), &Params);
				Context.CommandBuffer->Draw(6, 1, 0, 0);
			}
		});
	}

}
