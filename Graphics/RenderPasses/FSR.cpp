#include "RenderPasses.h"

namespace Columbus
{

	struct FSR1Parameters
	{
		iVector2 ImageSize;
		iVector2 MaxImageSize;
		iVector2 OutputSize;
		float Sharpening;
		int IsSharpeningPass;
		int IsHdr;
	};

	// TODO: support HDR input (apply tonemapping and inverse tonemapping)
	// TODO: manage descriptor sets properly (if it's used multiple times in the frame)
	// TODO: adding those screen-size and compute renderpasses seems repetative, create a common util for that task (requires a shader system)
	RenderGraphTextureRef ApplyFSR1(RenderGraph& Graph, RenderGraphTextureRef Texture, const TextureDesc2& DstDesc, iVector2 UpscaleTo, bool IsHdr, bool UseSharpening, float Sharpening)
	{
		COLUMBUS_ASSERT(DstDesc.Usage != TextureUsage::Sampled);
		COLUMBUS_ASSERT(DstDesc.Usage != TextureUsage::RenderTargetDepth);

		TextureDesc2 Desc = Graph.GetTextureDesc(Texture);

		iVector2 Size = iVector2(Desc.Width, Desc.Height);
		iVector2 DstSize = iVector2(DstDesc.Width, DstDesc.Height);
		iVector2 UpscaleSize = UpscaleTo;

		const char* TexName = Graph.GetTextureName(Texture);
		char ResultName[512]{ 0 };
		snprintf(ResultName, 512, "%s (FSR1 upscaled)", TexName);

		char SharpenedResultName[512]{ 0 };
		snprintf(SharpenedResultName, 512, "%s (FSR upscaled + sharpened)", TexName);

		RenderGraphTextureRef Result = Graph.CreateTexture(DstDesc, ResultName);

		const bool bUsePS = DstDesc.Usage == TextureUsage::RenderTargetColor;

		FSR1Parameters FsrParameters{
			.ImageSize = Size,
			.MaxImageSize = DstSize,
			.OutputSize = UpscaleSize,
			.Sharpening = Sharpening,
			.IsSharpeningPass = 0,
			.IsHdr = IsHdr ? 1 : 0,
		};

		// FSR1 EASU upscale pass
		if (bUsePS) // vertex/pixel shader path
		{
			RenderPassParameters Parameters;
			Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Result,  };
			Parameters.ViewportSize = UpscaleSize;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Texture, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);

			char PassName[256]{ 0 };
			snprintf(PassName, 256, "FSR1 (PS) %ix%i->%ix%i", Size.X, Size.Y, UpscaleSize.X, UpscaleSize.Y);

			Graph.AddPass(PassName, RenderGraphPassType::Raster, Parameters, Dependencies, [Texture, FsrParameters](RenderGraphContext& Context)
			{
				// TODO: shader system
				static GraphicsPipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					GraphicsPipelineDesc Desc;
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/FSR1/FSR1PS.csd");
					Desc.Name = "FSR1 (VS/PS)";
					Desc.rasterizerState.Cull = CullMode::No;
					Desc.blendState.RenderTargets = { RenderTargetBlendDesc() };
					Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
				}

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0); // TODO:
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Texture).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

				Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(FsrParameters), &FsrParameters);
				Context.CommandBuffer->Draw(3, 1, 0, 0);
			});
		}
		else // compute shader path
		{
			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Texture, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
			Dependencies.Write(Result, VK_ACCESS_SHADER_WRITE_BIT, VK_SHADER_STAGE_COMPUTE_BIT);

			char PassName[256]{ 0 };
			snprintf(PassName, 256, "FSR1 (CS) %ix%i->%ix%i", Size.X, Size.Y, UpscaleSize.X, UpscaleSize.Y);

			Graph.AddPass(PassName, RenderGraphPassType::Compute, Parameters, Dependencies, [Texture, Result, UpscaleSize, FsrParameters](RenderGraphContext& Context)
			{
				// TODO: shader system
				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/FSR1/FSR1CS.csd");
					Desc.Name = "FSR1 (CS)";

					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0); // TODO:
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Texture).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(Result).get());

				const int GroupSize = 16; // actual group size is 64x1, which is 8x8, but FSR works in 2x2 blocks
				const iVector2 GroupCount = (UpscaleSize + (GroupSize - 1)) / GroupSize;

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(FsrParameters), &FsrParameters);
				Context.CommandBuffer->Dispatch((u32)GroupCount.X, (u32)GroupCount.Y, 1);
			});
		}

		// FSR1 RCAS sharpening pass
		if (UseSharpening)
		{
			FsrParameters.IsSharpeningPass = 1;

			RenderGraphTextureRef SharpeningResult = Graph.CreateTexture(DstDesc, SharpenedResultName);

			if (bUsePS)
			{
				RenderPassParameters Parameters;
				Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, SharpeningResult, };
				Parameters.ViewportSize = DstSize;

				RenderPassDependencies Dependencies(Graph.Allocator);
				Dependencies.Read(Result, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);

				char PassName[256]{ 0 };
				snprintf(PassName, 256, "FSR1 (PS) sharpening (%.2f)", Sharpening);

				Graph.AddPass(PassName, RenderGraphPassType::Raster, Parameters, Dependencies, [Result, FsrParameters](RenderGraphContext& Context)
				{
					// TODO: shader system
					static GraphicsPipeline* Pipeline = nullptr;
					if (Pipeline == nullptr)
					{
						GraphicsPipelineDesc Desc;
						Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/FSR1/FSR1PS.csd");
						Desc.Name = "FSR1 (VS/PS)";
						Desc.rasterizerState.Cull = CullMode::No;
						Desc.blendState.RenderTargets = { RenderTargetBlendDesc() };
						Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
					}

					auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0); // TODO:
					Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Result).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

					Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
					Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
					Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(FsrParameters), &FsrParameters);
					Context.CommandBuffer->Draw(3, 1, 0, 0);
				});
			}

			Result = SharpeningResult;
		}

		return Result;
	}

}