#include "Common/Image/Image.h"
#include "Core/Core.h"
#include "Graphics/Core/Pipelines.h"
#include "Graphics/Core/Texture.h"
#include "Graphics/Core/Types.h"
#include "Graphics/Core/View.h"
#include "Graphics/GPUScene.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/Vulkan/TypeConversions.h"
#include "Profiling/Profiling.h"
#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterGBufferPass);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterLightingPass);

	IMPLEMENT_GPU_PROFILING_COUNTER("GBuffer", "RenderGraphGPU", GpuCounterGBufferPass);
	IMPLEMENT_GPU_PROFILING_COUNTER("Lighting pass", "RenderGraphGPU", GpuCounterLightingPass);

	struct PerObjectParameters
	{
		Matrix M,VP,VPPrev;
		// Matrix VPrev, PPrev;
		uint32_t ObjectId;
	};

	struct PerDecalParameters
	{
		Matrix Model, ModelInverse;
		Matrix VP;
	};

	struct GBufferLightingParameters
	{
		u32 LightsCount;
	};

	SceneTextures CreateSceneTextures(RenderGraph& Graph, const RenderView& View, HistorySceneTextures& History)
	{
		TextureDesc2 CommonDesc;
		CommonDesc.Usage = TextureUsage::RenderTargetColor;
		CommonDesc.Width = View.OutputSize.X;
		CommonDesc.Height = View.OutputSize.Y;
		CommonDesc.AddressU = TextureAddressMode::ClampToEdge;
		CommonDesc.AddressV = TextureAddressMode::ClampToEdge;
		CommonDesc.AddressW = TextureAddressMode::ClampToEdge;
		CommonDesc.MagFilter = TextureFilter2::Nearest;
		CommonDesc.MinFilter = TextureFilter2::Nearest;
		CommonDesc.MipFilter = TextureFilter2::Nearest;

		TextureDesc2 AlbedoDesc = CommonDesc;
		TextureDesc2 NormalDesc = CommonDesc;
		TextureDesc2 WPDesc = CommonDesc;
		TextureDesc2 RMDesc = CommonDesc;
		TextureDesc2 DSDesc = CommonDesc;
		TextureDesc2 VelocityDesc = CommonDesc;
				AlbedoDesc.Format = TextureFormat::RGBA8;
		NormalDesc.Format = TextureFormat::RGBA16F;
		WPDesc.Format = TextureFormat::RGBA32F;
		RMDesc.Format = TextureFormat::RG8;
		DSDesc.Format = TextureFormat::Depth24;
		DSDesc.Usage = TextureUsage::RenderTargetDepth;
		VelocityDesc.Format = TextureFormat::RG16F;
		
		SceneTextures Result { .History = History };
		Result.GBufferAlbedo = Graph.CreateTexture(AlbedoDesc, "GBufferAlbedo");
		Result.GBufferNormal = Graph.CreateTexture(NormalDesc, "GBufferNormal");
		Result.GBufferWP = Graph.CreateTexture(WPDesc, "GBufferWP");
		Result.GBufferRM = Graph.CreateTexture(RMDesc, "GBufferRM");
		Result.GBufferDS = Graph.CreateTexture(DSDesc, "GBufferDS");
		Result.Velocity = Graph.CreateTexture(VelocityDesc, "Velocity");
				return Result;
	}

	void RenderGBufferPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures)
	{
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferAlbedo };
		Parameters.ColorAttachments[1] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferNormal };
		Parameters.ColorAttachments[2] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferWP };
		Parameters.ColorAttachments[3] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferRM };
		Parameters.ColorAttachments[4] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.Velocity };
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferDS, AttachmentClearValue{ {}, 1.0f, 0 } };

		RenderPassDependencies Dependencies;

		Graph.AddPass("GBufferBasePass", RenderGraphPassType::Raster, Parameters, Dependencies, [View](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterGBufferPass, Context);

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
					RenderTargetBlendDesc(),
				};
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/GBufferPass.csd");

				Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.CommandBuffer->SetViewport(0, 0, View.OutputSize.X, View.OutputSize.Y, 0.0f, 1.0f);
			Context.CommandBuffer->SetScissor(0, 0, View.OutputSize.X, View.OutputSize.Y);
			Context.BindGPUScene(Pipeline);

			PerObjectParameters Parameters;

			for (int i = 0; i < Context.Scene->Meshes.size(); i++)
			{
				GPUSceneMesh& Mesh = Context.Scene->Meshes[i];
				Parameters.M = Matrix(1);
				Parameters.VP = View.CameraCur.GetViewProjection();
				// Parameters.P = View.CameraCur.GetProjectionMatrix();
				Parameters.VPPrev = View.CameraPrev.GetViewProjection();
				// Parameters.PPrev = View.CameraPrev.GetProjectionMatrix();
				Parameters.ObjectId = i;

				Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(Parameters), &Parameters);
				Context.CommandBuffer->Draw(Mesh.IndicesCount, 1, 0, 0);
			}
		});
	}

	void RenderGBufferDecals(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures)
	{
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferAlbedo };
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferDS, AttachmentClearValue{ {}, 1.0f, 0 } };

		RenderPassDependencies Dependencies;
		Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);

		Graph.AddPass("GBufferDecals", RenderGraphPassType::Raster, Parameters, Dependencies, [View, Textures](RenderGraphContext& Context)
		{
			static GraphicsPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				GraphicsPipelineDesc Desc;
				Desc.Name = "Decals";
				Desc.rasterizerState.Cull = CullMode::No;
				Desc.blendState.RenderTargets = {
					RenderTargetBlendDesc(),
				};

				Desc.depthStencilState.DepthEnable = false;
				Desc.depthStencilState.DepthWriteMask = false;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/Decals.csd");

				Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
			Context.CommandBuffer->SetViewport(0, 0, View.OutputSize.X, View.OutputSize.Y, 0.0f, 1.0f);
			Context.CommandBuffer->SetScissor(0, 0, View.OutputSize.X, View.OutputSize.Y);

			PerDecalParameters Parameters;

			for (int i = 0; i < Context.Scene->Decals.size(); i++)
			{
				GPUDecal& Decal = Context.Scene->Decals[i];
				Parameters.Model = Decal.Model;
				Parameters.ModelInverse = Decal.ModelInverse;
				Parameters.VP = View.CameraCur.GetViewProjection();

				auto& DecalDescriptorSet = Decal._DescriptorSets[Context.RenderData.CurrentPerFrameData];

				if (DecalDescriptorSet == NULL)
				{
					DecalDescriptorSet = Context.Device->CreateDescriptorSet(Pipeline, 1);
				}

				Context.Device->UpdateDescriptorSet(DecalDescriptorSet, 0, 0, Decal.Texture, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DecalDescriptorSet, 1, 0, Decal.Texture, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLER);

				Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 1, 1, &DecalDescriptorSet);
				Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(Parameters), &Parameters);
				Context.CommandBuffer->Draw(36, 1, 0, 0);
			}
		});
	}

	RenderGraphTextureRef RenderDeferredLightingPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& DeferredContext)
	{
		TextureDesc2 Desc {
			.Usage = TextureUsage::Storage,
			.Width = (uint32)View.OutputSize.X,
			.Height = (uint32)View.OutputSize.Y,
			.Format = TextureFormat::RGBA16F,
		};

		static const iVector2 GroupSize { 8, 8 };

		RenderGraphTextureRef LightingTexture = Graph.CreateTexture(Desc, "Lighting");

		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies;
		Dependencies.Read(Textures.GBufferAlbedo, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		for (GPULightRenderInfo& LightInfo : DeferredContext.LightRenderInfos)
		{
			Dependencies.Read(LightInfo.RTShadow, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		}
		Dependencies.Write(LightingTexture, VK_ACCESS_SHADER_WRITE_BIT, VK_SHADER_STAGE_COMPUTE_BIT);

		Graph.AddPass("DeferredLightingPass", RenderGraphPassType::Compute, Parameters, Dependencies, [View, LightingTexture, Textures, &DeferredContext](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterLightingPass, Context);

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

			GBufferLightingParameters Params {
				.LightsCount = (u32)Context.Scene->Lights.size()
			};

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferAlbedo).get());
			Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get());
			Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(LightingTexture).get());
			Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get());
			Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.Scene->LightsBuffer);

			auto ShadowsSet = Context.GetDescriptorSet(Pipeline, 1);
			for (int i = 0; i < DeferredContext.LightRenderInfos.size(); i++)
			{
				Context.Device->UpdateDescriptorSet(ShadowsSet, 0, i, Context.GetRenderGraphTexture(DeferredContext.LightRenderInfos[i].RTShadow).get());
			}

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 1, 1, &ShadowsSet);
			Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);

			const iVector2 GroupCount = (View.OutputSize + (GroupSize - 1)) / GroupSize;
			Context.CommandBuffer->Dispatch((u32)GroupCount.X, (u32)GroupCount.Y, 1);
		});

		return LightingTexture;
	}

	void CopyToSwapchain(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef Texture)
	{
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies;
		Dependencies.Read(Texture, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		Dependencies.Write(Graph.GetSwapchainTexture(), VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		Graph.AddPass("CopyToSwapchain", RenderGraphPassType::Compute, Parameters, Dependencies, [Texture, View](RenderGraphContext& Context)
		{
			SPtr<Texture2> Tonemapped = Context.GetRenderGraphTexture(Texture);
			Texture2* Swapchain = Context.RenderData.SwapchainImage;

			Context.CommandBuffer->TransitionImageLayout(Tonemapped.get(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			Context.CommandBuffer->TransitionImageLayout(Swapchain, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			Context.CommandBuffer->CopyImage(Tonemapped.get(), Swapchain, { 0,0,0 }, { 0,0,0 }, { View.OutputSize, 1 });
		});
	}

	void ExtractHistorySceneTextures(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, HistorySceneTextures& HistoryTextures)
	{
		Graph.ExtractTexture(Textures.GBufferDS, &HistoryTextures.Depth);
	}

	void RenderDeferred(RenderGraph& Graph, const RenderView& View, DeferredRenderContext& DeferredContext)
	{
		SceneTextures Textures = CreateSceneTextures(Graph, View, DeferredContext.History);

		DeferredContext.LightRenderInfos.clear();

		PrepareTiledLights(Graph, View);
		RenderGBufferPass(Graph, View, Textures);
		RenderGBufferDecals(Graph, View, Textures);

		RayTracedShadowsPass(Graph, View, Textures, DeferredContext);
		RenderIndirectLightingDDGI(Graph, View);
		RenderGraphTextureRef LightingTexture = RenderDeferredLightingPass(Graph, View, Textures, DeferredContext);
		RenderGraphTextureRef TonemappedImage = TonemapPass(Graph, View, LightingTexture);
		DebugOverlayPass(Graph, View, Textures, TonemappedImage);
		DebugUIPass(Graph, View, TonemappedImage);
		CopyToSwapchain(Graph, View, TonemappedImage);
		ExtractHistorySceneTextures(Graph, View, Textures, DeferredContext.History);
	}

}
