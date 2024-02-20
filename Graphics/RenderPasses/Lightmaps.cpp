#include "Graphics/Lightmaps.h"
#include "RenderPasses.h"

namespace Columbus
{

	struct LightmapRasterisationParameters
	{
		u64 VertexBuffer;
		u64 IndexBuffer;
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

	// see note in the declaration
	void PrepareAtlasForLightmapBaking(RenderGraph& Graph, LightmapSystem& System)
	{
		LightmapBakingSettings& Settings = System.BakingSettings;
		LightmapBakingRenderData& Data = System.BakingData;

		// TODO: unify format creation with GBuffer?
		TextureDesc2 CommonDesc;
		CommonDesc.Width = System.Atlas.Width;
		CommonDesc.Height = System.Atlas.Height;
		CommonDesc.Type = TextureType::Texture2D;
		CommonDesc.Usage = TextureUsage::RenderTargetColor;
		CommonDesc.AddressU = TextureAddressMode::ClampToEdge;
		CommonDesc.AddressV = TextureAddressMode::ClampToEdge;
		CommonDesc.AddressW = TextureAddressMode::ClampToEdge;
		CommonDesc.MagFilter = TextureFilter2::Nearest;
		CommonDesc.MinFilter = TextureFilter2::Nearest;
		CommonDesc.MipFilter = TextureFilter2::Nearest;

		TextureDesc2 NormalDesc = CommonDesc;
		TextureDesc2 PositionDesc = CommonDesc;
		TextureDesc2 ValidityDesc = CommonDesc;
		NormalDesc.Format = TextureFormat::RGBA16F;
		PositionDesc.Format = TextureFormat::RGBA32F;
		ValidityDesc.Format = TextureFormat::R8;

		System.BakingData.Normal = Graph.CreateTexture(NormalDesc, "LightmapBaking_Normal");
		System.BakingData.Position = Graph.CreateTexture(PositionDesc, "LightmapBaking_Posiiton");
		System.BakingData.Validity = Graph.CreateTexture(ValidityDesc, "LightmapBaking_Validity");

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, System.BakingData.Normal };
		Parameters.ColorAttachments[1] = RenderPassAttachment{ AttachmentLoadOp::Clear, System.BakingData.Position };
		Parameters.ColorAttachments[2] = RenderPassAttachment{ AttachmentLoadOp::Clear, System.BakingData.Validity };
		Parameters.ViewportSize = iVector2(System.Atlas.Width, System.Atlas.Height);

		iVector2 ViewportSize = Parameters.ViewportSize;

		RenderPassDependencies Dependencies(Graph.Allocator);

		Graph.AddPass("PrepareMeshForLightmapBaking", RenderGraphPassType::Raster, Parameters, Dependencies, [&System, ViewportSize](RenderGraphContext& Context)
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
				};
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/LightmapRasterisation.csd");

				Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.BindGPUScene(Pipeline);

			for (u32 i = 0; i < System.Meshes.size(); i++)
			{
				LightmapRasterisationParameters Parameters {
					.VertexBuffer = System.Meshes[i].VertexBuffer->GetDeviceAddress(),
					.IndexBuffer = System.Meshes[i].IndexBuffer->GetDeviceAddress(),
					.ObjectId = i,
				};

				GPUSceneMesh& Mesh = Context.Scene->Meshes[i];
				Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(Parameters), &Parameters);
				Context.CommandBuffer->Draw(Mesh.IndicesCount, 1, 0, 0);
			}
		});
	}

	void BakeLightmapPathTraced(RenderGraph& Graph, LightmapSystem& System)
	{
		LightmapBakingRenderData& Data = System.BakingData;
		LightmapBakingSettings& Settings = System.BakingSettings;

		if (Data.AccumulatedSamples >= Settings.RequestedSamples)
		{
			System.BakingRequested = false;
			return;
		}

		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(Data.Position, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		Dependencies.Read(Data.Normal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		Dependencies.Read(Data.Validity, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

		int TraceSamples = Math::Min(Settings.SamplesPerFrame, Settings.RequestedSamples - Data.AccumulatedSamples);

		Texture2* Lightmap = System.Atlas.Lightmap;
		u32 Width = System.Atlas.Width;
		u32 Height = System.Atlas.Height;

		Graph.AddPass("LightmapPathTrace", RenderGraphPassType::Compute, Parameters, Dependencies, [Data, Settings, TraceSamples, Lightmap, Width, Height](RenderGraphContext& Context)
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

			Context.CommandBuffer->TransitionImageLayout(Lightmap, VK_IMAGE_LAYOUT_GENERAL);

			auto Set = Context.GetDescriptorSet(Pipeline, 2);
			Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.Scene->TLAS); // TODO: move to unified scene set
			Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(Data.Position).get());
			Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(Data.Normal).get());
			Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.GetRenderGraphTexture(Data.Validity).get());
			Context.Device->UpdateDescriptorSet(Set, 4, 0, Lightmap);

			LightmapPathTracingParameters Parameters {
				.Random = rand(),
				.Bounces = Settings.Bounces,
				.AccumulatedSamples = Data.AccumulatedSamples,
				.RequestedSamples = Settings.RequestedSamples,
				.SamplesPerFrame = TraceSamples,
			};

			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &Set);
			Context.BindGPUScene(Pipeline);

			Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Parameters), &Parameters);
			Context.CommandBuffer->TraceRays(Pipeline, Width, Height, 1);
		});

		Data.AccumulatedSamples += TraceSamples;
	}

}
