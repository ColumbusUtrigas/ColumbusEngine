#include "Core/Core.h"
#include "Core/CVar.h"
#include "Graphics/Core/Types.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/Vulkan/VulkanShaderCompiler.h"
#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"

ConsoleVariable<int> CVar_FilmCurve("r.Tonemap.FilmCurve", "0 - ACES, 1 - AgX, 2 - Flim", 0);
ConsoleVariable<int> CVar_OutputTransform("r.Tonemap.OutputTransform", "0 - None, 1 - Rec.709, 2 - Rec.2020", 1);

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
		RenderGraphTextureRef TonemappedImage;
	};

	void ComputeColourGradingLUT(RenderGraph& Graph, TonemapTextures& Textures)
	{
		// TODO: external LUTs
		
		static const iVector3 LutResolution { 32, 32, 32 };
		static const iVector3 GroupSize { 8, 8, 8 };

		{
			TextureDesc2 Desc {
				.Type   = TextureType::Texture3D,
				.Usage  = TextureUsage::Storage,
				.Width  = (u32)LutResolution.X, .Height = (u32)LutResolution.Y, .Depth = (u32)LutResolution.Z,
				.Format = TextureFormat::RGBA16F
			};
			Textures.ColourGradingLUT = Graph.CreateTexture(Desc, "ColourGradingLUT");
		}

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Write(Textures.ColourGradingLUT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("ComputeColourGradingLUT", RenderGraphPassType::Compute, {}, Dependencies, [Textures](RenderGraphContext& Context)
		{
			static ComputePipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				ComputePipelineDesc Desc;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/ComputeColourGradingLUT.csd");
				Desc.Name = "ComputeColourGradingLUT";
				Pipeline = Context.Device->CreateComputePipeline(Desc);
			}

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Textures.ColourGradingLUT).get());

			const iVector3 GroupCount = LutResolution / GroupSize;;

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
			Context.CommandBuffer->Dispatch((u32)GroupCount.X, (u32)GroupCount.Y, (u32)GroupCount.Z);
		});
	}

	RenderGraphTextureRef TonemapPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef SceneTexture)
	{
		TonemapTextures Textures;
		ComputeColourGradingLUT(Graph, Textures);

		TextureDesc2 Desc = Graph.GetTextureDesc(SceneTexture);

		iVector2 Size(Desc.Width, Desc.Height);

		TextureDesc2 TonemapTextureDesc {
			.Usage = TextureUsage::RenderTargetColor,
			.Width = (u32)Size.X, .Height = (u32)Size.Y,
			.Format = TextureFormat::BGRA8SRGB,
			.AddressU = TextureAddressMode::ClampToEdge,
			.AddressV = TextureAddressMode::ClampToEdge,
			.AddressW = TextureAddressMode::ClampToEdge,
		};
		Textures.TonemappedImage = Graph.CreateTexture(TonemapTextureDesc, "Tonemapped");

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.TonemappedImage };
		Parameters.ViewportSize = Size;

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(Textures.ColourGradingLUT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		Dependencies.Read(SceneTexture, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		Graph.AddPass("Tonemap", RenderGraphPassType::Raster, Parameters, Dependencies, [SceneTexture, View, Size](RenderGraphContext& Context)
		{
			// TODO: shader system
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
				.FilmCurve       = (TonemapFilmCurve)CVar_FilmCurve.GetValue(),
				.OutputTransform = (TonemapOutputTransform)CVar_OutputTransform.GetValue(),
				.Resolution      = iVector2(Texture->GetDesc().Width, Texture->GetDesc().Height),
			};

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Texture.get());

			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
			Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Pixel, 0, sizeof(PushConstants), &PushConstants);
			Context.CommandBuffer->Draw(3, 1, 0, 0);
		});

		return Textures.TonemappedImage;
	}

}
