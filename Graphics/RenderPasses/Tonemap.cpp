#include "Core/Core.h"
#include "Graphics/Core/Types.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/Vulkan/VulkanShaderCompiler.h"
#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"

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

		iVector2 Resolution;
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
				ComputePipelineDesc Desc;

				ShaderStageDesc Stages[] = { ShaderStageDesc{ ShaderType::Compute, "main" } };
				Desc.Bytecode = CompileShaderPipelineFromSource_VK(ShaderSource, "ComputeColourGradingLUT", ShaderLanguage::GLSL, Stages, {});
				Desc.Name = "ComputeColourGradingLUT";

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

		Graph.AddPass("Tonemap", RenderGraphPassType::Raster, Parameters, Dependencies, [SceneTexture](RenderGraphContext& Context)
		{
			// TODO:
			// Context.GetGraphicsPipelineFromFile("Tonemap", Tonemap.glsl", "main", "main", ShaderLanguage::GLSL);

			static GraphicsPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				GraphicsPipelineDesc Desc;
				Desc.Name = "Tonemap";
				Desc.rasterizerState.Cull = CullMode::No;
				Desc.blendState.RenderTargets = { RenderTargetBlendDesc() };
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/Tonemap.csd");

				Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			SPtr<Texture2> Texture = Context.GetRenderGraphTexture(SceneTexture);

			TonemapParameters PushConstants {
				.FilmCurve       = TonemapFilmCurve::ACES,
				.OutputTransform = TonemapOutputTransform::Rec709,
				.Resolution      = iVector2(Texture->GetDesc().Width, Texture->GetDesc().Height),
			};

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Texture.get());

			// TODO: pipeline viewport size
			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
			Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Pixel, 0, sizeof(PushConstants), &PushConstants);
			Context.CommandBuffer->Draw(3, 1, 0, 0);
		});
	}

}
