#include "RenderPasses.h"

namespace Columbus
{

	struct LightmapRasterisationParameters
	{
		u32 ObjectId;
	};

	struct LightmapPathTracingParameters
	{
		int Random;
		int Bounces;
		int AccumulatedSamples;
		int RequestedSamples;
		int SamplesPerFrame;
	};

	LightmapBakingRenderData CreateLightmapBakingData(SPtr<DeviceVulkan> Device, LightmapBakingSettings Settings)
	{
		LightmapBakingRenderData BakingData {
			.Settings = Settings,
		};

		TextureDesc2 LightmapTextureDesc {
			.Usage = TextureUsage::StorageSampled,
			.Width = (u32)Settings.LightmapSize, .Height = (u32)Settings.LightmapSize,
			.Format = TextureFormat::RGBA16F,
		};

		BakingData.Lightmap = Device->CreateTexture(LightmapTextureDesc);
		Device->SetDebugName(BakingData.Lightmap, "Lightmap");

		return BakingData;
	}

	// see note in the declaration
	void PrepareMeshForLightmapBaking(RenderGraph& Graph, const RenderView& View, LightmapBakingRenderData& Data)
	{
		// TODO: unify format creation with GBuffer?
		TextureDesc2 CommonDesc;
		CommonDesc.Width = Data.Settings.LightmapSize;
		CommonDesc.Height = Data.Settings.LightmapSize;
		CommonDesc.Type = TextureType::Texture2D;
		CommonDesc.Usage = TextureUsage::RenderTargetColor;
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
		TextureDesc2 ValidityDesc = CommonDesc;
		AlbedoDesc.Format = TextureFormat::RGBA8;
		NormalDesc.Format = TextureFormat::RGBA16F;
		WPDesc.Format = TextureFormat::RGBA32F;
		RMDesc.Format = TextureFormat::RG8;
		ValidityDesc.Format = TextureFormat::R8;

		Data.Albedo = Graph.CreateTexture(AlbedoDesc, "LightmapBaking_Albedo");
		Data.Normal = Graph.CreateTexture(NormalDesc, "LightmapBaking_Normal");
		Data.WP = Graph.CreateTexture(WPDesc, "LightmapBaking_WP");
		Data.RM = Graph.CreateTexture(RMDesc, "LightmapBaking_RM");
		Data.Validity = Graph.CreateTexture(ValidityDesc, "LightmapBaking_Validity");

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Data.Albedo };
		Parameters.ColorAttachments[1] = RenderPassAttachment{ AttachmentLoadOp::Clear, Data.Normal };
		Parameters.ColorAttachments[2] = RenderPassAttachment{ AttachmentLoadOp::Clear, Data.WP };
		Parameters.ColorAttachments[3] = RenderPassAttachment{ AttachmentLoadOp::Clear, Data.RM };
		Parameters.ColorAttachments[4] = RenderPassAttachment{ AttachmentLoadOp::Clear, Data.Validity };
		Parameters.ViewportSize = iVector2(Data.Settings.LightmapSize, Data.Settings.LightmapSize);

		RenderPassDependencies Dependencies;

		u32 MeshIndex = Data.Settings.MeshIndex;
		int LightmapSize = Data.Settings.LightmapSize;

		char PassName[256]{0};
		snprintf(PassName, 256, "PrepareMeshForLightmapBaking (%i)", MeshIndex);
		Graph.AddPass(PassName, RenderGraphPassType::Raster, Parameters, Dependencies, [MeshIndex, LightmapSize](RenderGraphContext& Context)
		{
			// TODO: normal shader system damn it
			static GraphicsPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				GraphicsPipelineDesc Desc;
				Desc.Name = "LightmapRasterisation";
				Desc.rasterizerState.Cull = CullMode::No;
				Desc.blendState.RenderTargets = {
					RenderTargetBlendDesc(),
					RenderTargetBlendDesc(),
					RenderTargetBlendDesc(),
					RenderTargetBlendDesc(),
					RenderTargetBlendDesc(),
				};
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/LightmapRasterisation.csd");

				Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.CommandBuffer->SetViewport(0, 0, LightmapSize, LightmapSize, 0.0f, 1.0f);
			Context.CommandBuffer->SetScissor(0, 0, LightmapSize, LightmapSize);
			Context.BindGPUScene(Pipeline);

			LightmapRasterisationParameters Parameters {
				.ObjectId = MeshIndex
			};

			GPUSceneMesh& Mesh = Context.Scene->Meshes[MeshIndex];
			Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(Parameters), &Parameters);
			Context.CommandBuffer->Draw(Mesh.IndicesCount, 1, 0, 0);
		});
	}

	void BakeLightmapPathTraced(RenderGraph& Graph, LightmapBakingRenderData& Data)
	{
		if (Data.AccumulatedSamples >= Data.Settings.RequestedSamples)
		{
			return;
		}

		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies;
		Dependencies.Read(Data.WP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		Dependencies.Read(Data.Normal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		Dependencies.Read(Data.Validity, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

		int TraceSamples = Math::Min(Data.Settings.SamplesPerFrame, Data.Settings.RequestedSamples - Data.AccumulatedSamples);

		Graph.AddPass("LightmapPathTrace", RenderGraphPassType::Compute, Parameters, Dependencies, [Data, TraceSamples](RenderGraphContext& Context)
		{
			static RayTracingPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				RayTracingPipelineDesc Desc{};
				Desc.Name = "LightmapPathTracing";
				Desc.MaxRecursionDepth = 1;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/LightmapPathTracing.csd");
				Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
			}

			Context.CommandBuffer->TransitionImageLayout(Data.Lightmap, VK_IMAGE_LAYOUT_GENERAL);

			auto Set = Context.GetDescriptorSet(Pipeline, 6);
			Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.Scene->TLAS); // TODO: move to unified scene set
			Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(Data.WP).get());
			Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(Data.Normal).get());
			Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.GetRenderGraphTexture(Data.Validity).get());
			Context.Device->UpdateDescriptorSet(Set, 4, 0, Data.Lightmap);

			LightmapPathTracingParameters Parameters {
				.Random = rand(),
				.Bounces = Data.Settings.Bounces,
				.AccumulatedSamples = Data.AccumulatedSamples,
				.RequestedSamples = Data.Settings.RequestedSamples,
				.SamplesPerFrame = TraceSamples,
			};

			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 6, 1, &Set);
			Context.BindGPUScene(Pipeline);

			Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Parameters), &Parameters);
			Context.CommandBuffer->TraceRays(Pipeline, Data.Settings.LightmapSize, Data.Settings.LightmapSize, 1);
		});

		Data.AccumulatedSamples += TraceSamples;
	}

}
