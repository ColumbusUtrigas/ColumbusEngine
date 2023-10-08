#include "Core/Core.h"
#include "Graphics/Core/Types.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"

namespace Columbus
{

	// Image formation type
	enum class TonemapFilmCurve : u32
	{
		ACES,
		AgX,
		Flim
	};

	enum class TonemapOutputTransform : u32
	{
		None,
		Rec709,
		Rec2020
	};

	struct TonemapParameters
	{
		TonemapFilmCurve       FilmCurve;
		TonemapOutputTransform OutputTransform;
	};

	struct TonemapTextures
	{
		RenderGraphTextureRef ColourGradingLUT;
	};

	void ComputeColourGradingLUT(RenderGraph& Graph, TonemapTextures& Textures)
	{
		// TODO: external LUTs

		{
			TextureDesc2 Desc {
				.Type   = TextureType::Texture3D,
				.Usage  = TextureUsage::Storage,
				.Width  = 32, .Height = 32, .Depth = 32,
				.Format = TextureFormat::RGBA16F
			};
			Textures.ColourGradingLUT = Graph.CreateTexture(Desc, "ColourGradingLUT");
		}

		RenderPassDependencies Dependencies;
		Dependencies.Write(Textures.ColourGradingLUT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("ComputeColourGradingLUT", RenderGraphPassType::Compute, {}, Dependencies, [Textures](RenderGraphContext& Context)
		{
			static ComputePipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				auto ShaderSource = LoadShaderFile("ComputeColourGradingLUT.glsl");
				ComputePipelineDesc Desc {
					std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Compute, ShaderLanguage::GLSL),
					"ComputeColourGradingLUT"
				};
				Pipeline = Context.Device->CreateComputePipeline(Desc);
			}

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Textures.ColourGradingLUT).get());

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
			Context.CommandBuffer->Dispatch(32, 32, 32);
		});
	}

	void TonemapPass(RenderGraph& Graph, RenderGraphTextureRef SceneTexture)
	{
		TonemapTextures Textures;
		ComputeColourGradingLUT(Graph, Textures);

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Graph.GetSwapchainTexture() };

		RenderPassDependencies Dependencies;
		Dependencies.Read(Textures.ColourGradingLUT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		Dependencies.Read(SceneTexture, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		TonemapParameters PushConstants {
			.FilmCurve       = TonemapFilmCurve::ACES,
			.OutputTransform = TonemapOutputTransform::Rec709
		};

		Graph.AddPass("Tonemap", RenderGraphPassType::Raster, Parameters, Dependencies, [PushConstants, SceneTexture](RenderGraphContext& Context)
		{
			// TODO:
			// Context.GetGraphicsPipelineFromFile("Tonemap", Tonemap.glsl", "main", "main", ShaderLanguage::GLSL);

			static GraphicsPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				auto ShaderSource = LoadShaderFile("Tonemap.glsl");
				GraphicsPipelineDesc Desc;
				Desc.Name = "Tonemap";
				Desc.rasterizerState.Cull = CullMode::No;
				Desc.VS = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Vertex, ShaderLanguage::GLSL);
				Desc.PS = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Pixel, ShaderLanguage::GLSL);
				Desc.blendState.RenderTargets = { RenderTargetBlendDesc() };
				Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(SceneTexture).get());

			// TODO: pipeline viewport size
			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
			// Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Pixel, 0, sizeof(PushConstants), &PushConstants);
			Context.CommandBuffer->Draw(3, 1, 0, 0);
		});
	}

}
