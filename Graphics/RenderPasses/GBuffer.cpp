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

// TODO: remove
#include <Lib/imgui/imgui.h>

namespace Columbus
{

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterGBufferPass);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterLightingPass);

	IMPLEMENT_GPU_PROFILING_COUNTER("GBuffer", "RenderGraphGPU", GpuCounterGBufferPass);
	IMPLEMENT_GPU_PROFILING_COUNTER("Lighting pass", "RenderGraphGPU", GpuCounterLightingPass);

	struct PerObjectParameters
	{
		u32 ObjectId;
	};

	struct PerDecalParameters
	{
		Matrix Model, ModelInverse;
		Matrix VP;
	};

	SceneTextures CreateSceneTextures(RenderGraph& Graph, const RenderView& View, HistorySceneTextures& History)
	{
		TextureDesc2 CommonDesc;
		CommonDesc.Usage = TextureUsage::RenderTargetColor;
		CommonDesc.Width = View.RenderSize.X;
		CommonDesc.Height = View.RenderSize.Y;
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
		TextureDesc2 EmissiveDesc = CommonDesc;
		TextureDesc2 DSDesc = CommonDesc;
		TextureDesc2 VelocityDesc = CommonDesc;
		TextureDesc2 LightmapDesc = CommonDesc;
		AlbedoDesc.Format = TextureFormat::RGBA8;
		NormalDesc.Format = TextureFormat::RGBA16F;
		WPDesc.Format = TextureFormat::RGBA32F;
		RMDesc.Format = TextureFormat::RG8;
		EmissiveDesc.Format = TextureFormat::R11G11B10F;
		DSDesc.Format = TextureFormat::Depth32F;
		DSDesc.Usage = TextureUsage::RenderTargetDepth;
		VelocityDesc.Format = TextureFormat::RG16F;
		LightmapDesc.Format = TextureFormat::RGBA16F;
		
		SceneTextures Result { .History = History };
		Result.GBufferAlbedo = Graph.CreateTexture(AlbedoDesc, "GBufferAlbedo");
		Result.GBufferNormal = Graph.CreateTexture(NormalDesc, "GBufferNormal");
		Result.GBufferWP = Graph.CreateTexture(WPDesc, "GBufferWP");
		Result.GBufferRM = Graph.CreateTexture(RMDesc, "GBufferRM");
		Result.GBufferEmissive = Graph.CreateTexture(EmissiveDesc, "GBufferEmissive");
		Result.GBufferDS = Graph.CreateTexture(DSDesc, "GBufferDS");
		Result.Velocity = Graph.CreateTexture(VelocityDesc, "Velocity");
		Result.Lightmap = Graph.CreateTexture(LightmapDesc, "Lightmap");
		Result.VolumetricFog = Result.GBufferAlbedo;

		// history textures
		Graph.CreateHistoryTexture(&Result.History.Depth, DSDesc, "GBufferDS History");
		Graph.CreateHistoryTexture(&Result.History.Normals, NormalDesc, "GBufferNormal History");
		Graph.CreateHistoryTexture(&Result.History.RoughnessMetallic, RMDesc, "GBufferRM History");

		return Result;
	}

	void ExtractHistorySceneTextures(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, HistorySceneTextures& HistoryTextures)
	{
		Graph.ExtractTexture(Textures.GBufferDS, &HistoryTextures.Depth);
		Graph.ExtractTexture(Textures.GBufferRM, &HistoryTextures.RoughnessMetallic);
		Graph.ExtractTexture(Textures.GBufferNormal, &HistoryTextures.Normals);
	}

	void HistorySceneTextures::Destroy(SPtr<DeviceVulkan> Device)
	{
		Device->DestroyTexture(Depth);
		Device->DestroyTexture(RoughnessMetallic);
		Device->DestroyTexture(Normals);
		Device->DestroyTexture(VolumetricFogFroxels);
		Device->DestroyTexture(TAAHistory);
		Device->DestroyTexture(RTReflectionsRadiance);
		Device->DestroyTexture(RTReflectionsSampleCount);
		RTGI_History.Destroy(Device);
		Depth = nullptr;
		RoughnessMetallic = nullptr;
		Normals = nullptr;
		VolumetricFogFroxels = nullptr;
		TAAHistory = nullptr;
		RTReflectionsRadiance = nullptr;
		RTReflectionsSampleCount = nullptr;
	}

	void RenderGBufferPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures)
	{
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferAlbedo };
		Parameters.ColorAttachments[1] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferNormal };
		Parameters.ColorAttachments[2] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferWP };
		Parameters.ColorAttachments[3] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferRM };
		Parameters.ColorAttachments[4] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferEmissive };
		Parameters.ColorAttachments[5] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.Velocity };
		Parameters.ColorAttachments[6] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.Lightmap };
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferDS, AttachmentClearValue{ {}, 1.0f, 0 } };
		Parameters.ViewportSize = View.RenderSize;

		RenderPassDependencies Dependencies(Graph.Allocator);

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
					RenderTargetBlendDesc(),
					RenderTargetBlendDesc(),
				};
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/GBufferPass.csd");

				Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.BindGPUScene(Pipeline, false);

			for (int i = 0; i < Context.Scene->Meshes.Size(); i++)
			{
				const GPUSceneMesh& Mesh = Context.Scene->Meshes.Data()[i];
				PerObjectParameters Parameters;
				Parameters.ObjectId = i;

				Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex, 0, sizeof(Parameters), &Parameters);
				Context.CommandBuffer->Draw(Mesh.MeshResource->IndicesCount, 1, 0, 0);
			}
		});
	}

	void RenderGBufferDecals(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures)
	{
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferAlbedo };
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferDS, AttachmentClearValue{ {}, 1.0f, 0 } };
		Parameters.ViewportSize = View.RenderSize;

		RenderPassDependencies Dependencies(Graph.Allocator);
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
					RenderTargetBlendDesc {
						.BlendEnable = true,
						.SrcBlend = Blend::SrcAlpha,
						.DestBlend = Blend::InvSrcAlpha,
					},
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

			PerDecalParameters Parameters;

			for (int i = 0; i < Context.Scene->Decals.Size(); i++)
			{
				GPUDecal& Decal = ((GPUDecal*)Context.Scene->Decals.Data())[i]; // TODO: nasty
				Parameters.Model = Decal.Model;
				Parameters.ModelInverse = Decal.ModelInverse;
				Parameters.VP = View.CameraCur.GetViewProjection();

				if (!Decal.Texture)
					continue;

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
			.Usage = TextureUsage::StorageSampled | TextureUsage::RenderTargetColor,
			.Width = (uint32)View.RenderSize.X,
			.Height = (uint32)View.RenderSize.Y,
			.Format = TextureFormat::RGBA16F,
		};

		static const iVector2 GroupSize { 8, 8 };

		RenderGraphTextureRef LightingTexture = Graph.CreateTexture(Desc, "Lighting");

		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(Textures.GBufferAlbedo, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		Dependencies.Read(Textures.GBufferEmissive, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		Dependencies.Read(Textures.Lightmap, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT);
		Dependencies.Read(Textures.RTReflections, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT); // TODO: optional
		Dependencies.Read(Textures.RTGI, VK_ACCESS_SHADER_READ_BIT, VK_SHADER_STAGE_COMPUTE_BIT); // TODO: optional
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

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferAlbedo).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(Textures.GBufferEmissive).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, Context.GetRenderGraphTexture(Textures.Lightmap).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 6, 0, Context.GetRenderGraphTexture(Textures.RTReflections).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 7, 0, Context.GetRenderGraphTexture(Textures.RTGI).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 8, 0, Context.GetRenderGraphTexture(LightingTexture).get());
			Context.Device->UpdateDescriptorSet(DescriptorSet, 9, 0, Context.Scene->LightsBuffer);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 10, 0, Context.Scene->SceneBuffer);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 11, 0, Context.Scene->LTC_1, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 12, 0, Context.Scene->LTC_2, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 13, 0, Context.Device->GetStaticSampler());

			auto ShadowsSet = Context.GetDescriptorSet(Pipeline, 1);
			// TODO: support lights not having shadows
			// TODO: support shadowmaps/hybrid shadows
			for (int i = 0; i < DeferredContext.LightRenderInfos.size(); i++)
			{
				Context.Device->UpdateDescriptorSet(ShadowsSet, 0, i, Context.GetRenderGraphTexture(DeferredContext.LightRenderInfos[i].RTShadow).get());
			}

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 1, 1, &ShadowsSet);

			const iVector2 GroupCount = (View.RenderSize + (GroupSize - 1)) / GroupSize;
			Context.CommandBuffer->Dispatch((u32)GroupCount.X, (u32)GroupCount.Y, 1);
		});

		return LightingTexture;
	}

	struct MemsetTextureParameters
	{
		Vector4  Value;
		iVector2 TextureSize;
	};

	void ShaderMemsetTexture(RenderGraph& Graph, RenderGraphTextureRef Texture, Vector4 Value)
	{
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Write(Texture, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("MemsetTexture", RenderGraphPassType::Compute, Parameters, Dependencies, [Texture, Value](RenderGraphContext& Context)
		{
			SPtr<Texture2> Tex = Context.GetRenderGraphTexture(Texture);
			Context.CommandBuffer->MemsetTexture(Tex.get(), Value);
		});
	}

	// TODO: find a way to do it without this pass
	static void TransitionImagePass(RenderGraph& Graph, RenderGraphTextureRef Texture)
	{
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(Texture, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		Graph.AddPass("TransitionImage", RenderGraphPassType::Compute, Parameters, Dependencies, [Texture](RenderGraphContext& Context)
		{
			Context.CommandBuffer->TransitionImageLayout(Context.GetRenderGraphTexture(Texture).get(), VK_IMAGE_LAYOUT_GENERAL);
		});
	}

	void PrepareDeferredView(RenderView& View)
	{
		View.CameraCurUnjittered = View.CameraCur;
		View.CameraPrevUnjittered = View.CameraPrev;

		if (View.DeferredSettings.ApplyFSR1)
		{
			View.RenderSize = iVector2((int)(View.OutputSize.X * View.DeferredSettings.RenderResolution), (int)(View.OutputSize.Y * View.DeferredSettings.RenderResolution));
		}
		else
		{
			View.RenderSize = View.OutputSize;
		}

		if (View.DeferredSettings.ApplyTAA)
		{
			Antialiasing::ApplyJitter(View);
		}
	}

	RenderGraphTextureRef RenderDeferred(RenderGraph& Graph, RenderView& View, DeferredRenderContext& DeferredContext)
	{
		DeferredContext.VisualisationMode = View.DeferredSettings.VisualisationMode;

		SceneTextures Textures = CreateSceneTextures(Graph, View, DeferredContext.History);

		DeferredContext.LightRenderInfos.clear();

		RenderPrepareSkyLut(Graph, View, Textures, DeferredContext);
		PrepareTiledLights(Graph, View);
		RenderGBufferPass(Graph, View, Textures);
		RenderGBufferDecals(Graph, View, Textures);

		// TODO: only if it's needed
		RadianceCache::TraceRadianceCache(Graph, View, Textures.RadianceCache);

		RayTracedShadowsPass(Graph, View, Textures, DeferredContext);
		RayTracedReflectionsPass(Graph, View, Textures, DeferredContext);

		switch (View.DeferredSettings.GlobalIlluminationMode)
		{
		case EDeferredGlobalIlluminationMode::RTGI:
			RayTracedGlobalIlluminationPass(Graph, View, Textures, DeferredContext);
			break;

		case EDeferredGlobalIlluminationMode::DDGI:
			RenderIndirectLightingDDGI(Graph, View, Textures, DeferredContext);
			break;

		case EDeferredGlobalIlluminationMode::IV:
			// TODO: no hardcode for the volumes
			RenderApplyIrradianceProbes(Graph, View, Textures, Graph.Scene->IrradianceVolumes[0]);
			break;
		case EDeferredGlobalIlluminationMode::None:
		{
			TextureDesc2 Desc{
				.Usage = TextureUsage::Storage | TextureUsage::Sampled,
				.Width = (u32)View.RenderSize.X,
				.Height = (u32)View.RenderSize.Y,
				.Format = TextureFormat::RGBA16F,
			};

			Textures.RTGI = Graph.CreateTexture(Desc, "RayTracedGI");
			ShaderMemsetTexture(Graph, Textures.RTGI, {});
		}
		}

		Textures.FinalBeforeTonemap = RenderDeferredLightingPass(Graph, View, Textures, DeferredContext);
		RenderDeferredSky(Graph, View, Textures, DeferredContext, Textures.FinalBeforeTonemap);
		Textures.FinalBeforeTonemap = RenderVolumetricFog(Graph, View, Textures, Textures.FinalBeforeTonemap);
		RenderDeferredTransparency(Graph, View, Textures, DeferredContext);

		// TODO: find a way to apply DoF after TAA
		Textures.FinalBeforeTonemap = FFX::DispatchDepthOfFieldRG(Graph, View, Textures, DeferredContext.FFX);

		if (View.DeferredSettings.ApplyTAA)
		{
			Textures.FinalBeforeTonemap = Antialiasing::RenderTAA(Graph, View, Textures);
		}

		RenderGraphTextureRef TonemappedImage = TonemapPass(Graph, View, Textures.FinalBeforeTonemap);
		Textures.FinalAfterTonemap = TonemappedImage;
		ScreenshotPass(Graph, View, View.ScreenshotHDR ? Textures.FinalBeforeTonemap : TonemappedImage);

		DebugOverlayPass(Graph, View, Textures, TonemappedImage);

		// debug visualisation modes
		if (DeferredContext.VisualisationMode != EDeferredRenderVisualisationMode::Final)
		{
			if (DeferredContext.VisualisationMode == EDeferredRenderVisualisationMode::RadianceCache)
			{
				TonemappedImage = RadianceCache::VisualiseRadianceCache(Graph, View, Textures.RadianceCache, Textures.GBufferWP);
			}
			else
			{
				TonemappedImage = DebugVisualisationPass(Graph, View, Textures, DeferredContext);
			}
		}

		if (View.DeferredSettings.ApplyFSR1)
		{
			TextureDesc2 FsrUpscaleDesc = Graph.GetTextureDesc(TonemappedImage);
			//FsrUpscaleDesc.Usage = TextureUsage::StorageSampled;
			FsrUpscaleDesc.Width = View.OutputSize.X;
			FsrUpscaleDesc.Height = View.OutputSize.Y;

			const iVector2 UpscaleTo = View.OutputSize;
			const float UpscaleFactor = 1.0f / View.DeferredSettings.RenderResolution;
			const bool IsHdr = true;

			TonemappedImage = ApplyFSR1(Graph, TonemappedImage, FsrUpscaleDesc, UpscaleTo, IsHdr, View.DeferredSettings.ApplyFSR1Sharpening, View.DeferredSettings.FSR1Sharpening);
		}

		RenderUIPass(Graph, View, TonemappedImage);

		TransitionImagePass(Graph, TonemappedImage);
		ExtractHistorySceneTextures(Graph, View, Textures, DeferredContext.History);

		return TonemappedImage;
	}

}
