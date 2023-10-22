#include "Core/Core.h"
#include "Graphics/Core/Pipelines.h"
#include "Graphics/Core/Types.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"

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

	void RenderGBufferPass(RenderGraph& Graph, const Camera& MainCamera, SceneTextures& Textures, const iVector2& WindowSize)
	{
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferAlbedo };
		Parameters.ColorAttachments[1] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferNormal };
		Parameters.ColorAttachments[2] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferWP };
		Parameters.ColorAttachments[3] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferRM };
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferDS, AttachmentClearValue{ {}, 1.0f, 0 } };

		RenderPassDependencies Dependencies;

		Graph.AddPass("GBufferBasePass", RenderGraphPassType::Raster, Parameters, Dependencies, [&MainCamera, WindowSize](RenderGraphContext& Context)
		{
			// TODO: refactor, create a proper shader system
			static GraphicsPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				GraphicsPipelineDesc Desc;
				Desc.Name = "GBufferPass";
				Desc.rasterizerState.Cull = CullMode::No;
				Desc.blendState.RenderTargets = {
					RenderTargetBlendDesc(),
					RenderTargetBlendDesc(),
					RenderTargetBlendDesc(),
					RenderTargetBlendDesc(),
				};
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/GBufferPass.csd");

				Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.CommandBuffer->SetViewport(0, 0, WindowSize.X, WindowSize.Y, 0.0f, 1.0f);
			Context.CommandBuffer->SetScissor(0, 0, WindowSize.X, WindowSize.Y);
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

	RenderGraphTextureRef RenderDeferredLightingPass(RenderGraph& Graph, const iVector2& WindowSize, RenderGraphTextureRef ShadowTexture, const SceneTextures& Textures)
	{
		TextureDesc2 Desc {
			.Usage = TextureUsage::Storage,
			.Width = (uint32)WindowSize.X,
			.Height = (uint32)WindowSize.Y,
			.Format = TextureFormat::RGBA16F,
		};

		RenderGraphTextureRef LightingTexture = Graph.CreateTexture(Desc, "Lighting");

		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies;
		Dependencies.Read(Textures.GBufferAlbedo, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		Dependencies.Read(ShadowTexture, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		Dependencies.Write(LightingTexture, VK_ACCESS_SHADER_WRITE_BIT, VK_SHADER_STAGE_COMPUTE_BIT);

		Graph.AddPass("DeferredLightingPass", RenderGraphPassType::Compute, Parameters, Dependencies, [WindowSize, LightingTexture, ShadowTexture, Textures](RenderGraphContext& Context)
		{
			// TODO: shader system
			static ComputePipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				ComputePipelineDesc Desc;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/GBufferLightingPass.csd");
				Desc.Name = "DeferredLightingShader";
				ReflectCompiledShaderData(Desc.Bytecode);

				Pipeline = Context.Device->CreateComputePipeline(Desc);
			}

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferAlbedo).get());
			Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get());
			Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(LightingTexture).get());
			Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.GetRenderGraphTexture(ShadowTexture).get());

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);

			Context.CommandBuffer->Dispatch(WindowSize.X, WindowSize.Y, 1);
		});

		return LightingTexture;
	}

	void RenderDeferred(RenderGraph& Graph, const Camera& MainCamera, const iVector2& WindowSize)
	{
		SceneTextures Textures = CreateSceneTextures(Graph, WindowSize);

		RenderGBufferPass(Graph, MainCamera, Textures, WindowSize);

		RenderGraphTextureRef ShadowTexture = RayTracedShadowsPass(Graph, Textures, WindowSize);
		RenderGraphTextureRef LightingTexture = RenderDeferredLightingPass(Graph, WindowSize, ShadowTexture, Textures);
		// DebugOverlayPass(Graph, LightingTexture);
		TonemapPass(Graph, LightingTexture, WindowSize);
	}

}
