#include "Core/Core.h"
#include "Graphics/Core/Types.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	struct PerObjectParameters
	{
		Matrix M,V,P;
		uint32_t ObjectId;
	};

	SceneTextures CreateSceneTextures(RenderGraph& Graph, const iVector2& WindowSize)
	{
		TextureDesc2 CommonDesc;
		CommonDesc.Usage = TextureUsage::RenderTargetColor;
		CommonDesc.Width = WindowSize.X;
		CommonDesc.Height = WindowSize.Y;

		TextureDesc2 AlbedoDesc = CommonDesc;
		TextureDesc2 NormalDesc = CommonDesc;
		TextureDesc2 WPDesc = CommonDesc;
		TextureDesc2 RMDesc = CommonDesc;
		TextureDesc2 DSDesc = CommonDesc;
		AlbedoDesc.Format = TextureFormat::RGBA8;
		NormalDesc.Format = TextureFormat::RGBA16F;
		WPDesc.Format = TextureFormat::RGBA32F;
		RMDesc.Format = TextureFormat::RG8;
		DSDesc.Format = TextureFormat::Depth24;
		DSDesc.Usage = TextureUsage::RenderTargetDepth;

		SceneTextures Result;
		Result.GBufferAlbedo = Graph.CreateTexture(AlbedoDesc, "GBufferAlbedo");
		Result.GBufferNormal = Graph.CreateTexture(NormalDesc, "GBufferNormal");
		Result.GBufferWP = Graph.CreateTexture(WPDesc, "GBufferWP");
		Result.GBufferRM = Graph.CreateTexture(RMDesc, "GBufferRM");
		Result.GBufferDS = Graph.CreateTexture(DSDesc, "GBufferDS");
		return Result;
	}

	void RenderGBufferPass(RenderGraph& Graph, const Camera& MainCamera, SceneTextures& Textures)
	{
		// TODO: TextureRef and sync?
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferAlbedo };
		Parameters.ColorAttachments[1] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferNormal };
		Parameters.ColorAttachments[2] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferWP };
		Parameters.ColorAttachments[3] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferRM };
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferDS, AttachmentClearValue{ {}, 1.0f, 0 } };

		RenderPassDependencies Dependencies;
		Dependencies.Write(Textures.GBufferAlbedo, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		Graph.AddPass("GBufferBasePass", RenderGraphPassType::Raster, Parameters, {}, [&MainCamera](RenderGraphContext& Context)
		{
			// TODO: refactor, create a proper shader system
			static GraphicsPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
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
		});
	}

	void RayTracedShadowsPass(RenderGraph& Graph)
	{
	}
	void RenderDeferredLightingPass(RenderGraph& Graph)
	{
	}

	void RenderDeferred(RenderGraph& Graph, const Camera& MainCamera, const iVector2& WindowSize)
	{
		SceneTextures Textures = CreateSceneTextures(Graph, WindowSize);
		RenderGBufferPass(Graph, MainCamera, Textures);
		TonemapPass(Graph, Textures.GBufferAlbedo); // TODO: lighting pass

		// RenderPassParameters Parameters;
		// Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Graph.GetSwapchainTexture() };

		// RenderPassDependencies Dependencies;
		// Dependencies.Add(Textures.GBufferAlbedo);

		// Graph.AddPass("Final", RenderGraphPassType::Raster, Parameters, Dependencies, [](RenderGraphContext& Context)
		// {

		// });
	}

}
