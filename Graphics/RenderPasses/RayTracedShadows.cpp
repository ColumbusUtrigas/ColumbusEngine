#include "Common/Image/Image.h"
#include "Core/CVar.h"
#include "Graphics/Core/Pipelines.h"
#include "Graphics/Core/Types.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"
#include "imgui.h"
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	ConsoleVariable<float> Cvar_Angle("r.RTShadows.Angle", "Controls angle of randomisation of shadow rays", 0.0f);

	struct RTShadowParams
	{
		Vector3 Direction;
		float Angle;
		float Random;
		bool ValidHistory;
	};

	struct RTShadowDenoiserPrepareParams
	{
		iVector2 BufferDimensions;
		iVector2 PackedBufferDimensions;
	};

	struct RTShadowDenoiserTileClassificationParams
	{
		Matrix InvViewProjectionMatrix;
		Matrix InvProjectionMatrix;
		Matrix ReprojectionMatrix;
		Vector3 CameraPosition;
		float _pad;
		iVector2 BufferDimensions;
		iVector2 PackedBufferDimensions;
		int FirstFrame;
	};

	struct RTShadowDenoiserFilterParams
	{
		Matrix InvProjectionMatrix;
		iVector2 BufferDimensions;
		u32 PassIndex;
		u32 StepSize;
	};

	RenderGraphTextureRef DenoiseRayTracedShadow(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, RenderGraphTextureRef Shadow)
	{
		iVector2 ShadowSize = View.OutputSize;
		// 8x4 tiles
		iVector2 TilesSize = iVector2((View.OutputSize.X + 7) / 8, (View.OutputSize.Y + 3) / 4);

		RenderGraphTextureRef RTShadowTiles;
		RenderGraphTextureRef Moments;
		RenderGraphTextureRef Metadata;
		RenderGraphTextureRef ReprojectionResult;
		RenderGraphTextureRef DenoisedResult;

		// TODO: define lifetime, refactor
		static SPtr<Texture2> ShadowHistory;
		static SPtr<Texture2> MomentsHistory;

		// Prepare
		{
			TextureDesc2 Desc {
				.Usage = TextureUsage::Storage,
				.Width = (u32)TilesSize.X,
				.Height = (u32)TilesSize.Y,
				.Format = TextureFormat::R32UInt,
			};
			RTShadowTiles = Graph.CreateTexture(Desc, "RayTracedShadowTiles (8x4)");

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies;
			Dependencies.Read(Shadow, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(RTShadowTiles, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("ShadowDenoisePrepare", RenderGraphPassType::Compute, Parameters, Dependencies, [TilesSize, Shadow, RTShadowTiles, ShadowSize](RenderGraphContext& Context)
			{
				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "DenoiserShadowsPrepare";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedShadows/DenoiserShadowsPrepare.csd");

					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				RTShadowDenoiserPrepareParams Params {
					.BufferDimensions = ShadowSize,
					.PackedBufferDimensions = TilesSize,
				};

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Shadow).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(RTShadowTiles).get());

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				//Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.CommandBuffer->Dispatch((u32)ShadowSize.X, (u32)ShadowSize.Y, 1);
			});
		}

		// Tile classification
		{
			TextureDesc2 MomentsDesc { .Usage = TextureUsage::Storage, .Width = (u32)TilesSize.X, .Height = (u32)TilesSize.Y, .Format = TextureFormat::R11G11B10F, };
			TextureDesc2 MetadataDesc { .Usage = TextureUsage::Storage, .Width = (u32)TilesSize.X, .Height = (u32)TilesSize.Y, .Format = TextureFormat::R32UInt, };
			TextureDesc2 ReprojectionResultDesc { .Usage = TextureUsage::Storage, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::RG16F, };
			// TODO: moments history
			// TODO: shadow history
			// TODO: move history textures to context

			Moments = Graph.CreateTexture(MomentsDesc, "RayTracedShadowsMoments");
			Metadata = Graph.CreateTexture(MetadataDesc, "RayTracedShadowsMetadata");
			ReprojectionResult = Graph.CreateTexture(ReprojectionResultDesc, "RayTracedShadowsReprojectionResult");

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies;
			Dependencies.Read(RTShadowTiles, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.Velocity, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Dependencies.Write(Moments, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Metadata, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(ReprojectionResult, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("ShadowDenoiseTileClassification", RenderGraphPassType::Compute, Parameters, Dependencies,
			[RTShadowTiles, Textures, ShadowSize, TilesSize, View, Moments, Metadata, ReprojectionResult](RenderGraphContext& Context)
			{
				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "DenoiserShadowsTileClassification";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedShadows/DenoiserShadowsTileClassification.csd");

					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(RTShadowTiles).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				if (Textures.History.Depth)
				{
					Context.CommandBuffer->TransitionImageLayout(Textures.History.Depth.get(), VK_IMAGE_LAYOUT_GENERAL);
					Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Textures.History.Depth.get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				}
				Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(Textures.Velocity).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

				Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, Context.GetRenderGraphTexture(ReprojectionResult).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 6, 0, Context.GetRenderGraphTexture(Moments).get());
				if (MomentsHistory)
					Context.Device->UpdateDescriptorSet(DescriptorSet, 7, 0, MomentsHistory.get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 8, 0, Context.GetRenderGraphTexture(Metadata).get());
				if (ShadowHistory)
					Context.Device->UpdateDescriptorSet(DescriptorSet, 9, 0, ShadowHistory.get());


				Matrix InvViewProjection = View.CameraCur.GetViewProjection().GetInverted();
				Matrix InvProjection = View.CameraCur.GetProjectionMatrix().GetInverted();

				RTShadowDenoiserTileClassificationParams Params {
					.InvViewProjectionMatrix = InvViewProjection,
					.InvProjectionMatrix = InvProjection,
					.ReprojectionMatrix = InvViewProjection * View.CameraPrev.GetViewProjection(),
					.CameraPosition = View.CameraCur.Pos,
					.BufferDimensions = ShadowSize,
					.PackedBufferDimensions = TilesSize,
					.FirstFrame = ShadowHistory == nullptr,
				};

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.CommandBuffer->Dispatch((u32)ShadowSize.X, (u32)ShadowSize.Y, 1);
			});			
		}

		// Filter
		{
			TextureDesc2 ResultDesc { .Usage = TextureUsage::Storage, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::R8, };
			DenoisedResult = Graph.CreateTexture(ResultDesc, "ShadowDenoised");

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies;
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(ReprojectionResult, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Metadata, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(DenoisedResult, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("ShadowDenoiseFilter", RenderGraphPassType::Compute, Parameters, Dependencies, [View, Textures, ReprojectionResult, Metadata, DenoisedResult, ShadowSize](RenderGraphContext& Context)
			{
				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "DenoiserShadowsFilter";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedShadows/DenoiserShadowsFilter.csd");

					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				RTShadowDenoiserFilterParams Params {
					.InvProjectionMatrix = View.CameraCur.GetProjectionMatrix().GetInverted(),
					.BufferDimensions = ShadowSize,
					.PassIndex = 0,
					.StepSize = 5, // ????
				};

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(ReprojectionResult).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.GetRenderGraphTexture(Metadata).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(DenoisedResult).get());

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->Dispatch((u32)ShadowSize.X, (u32)ShadowSize.Y, 1);
			});
		}

		Graph.ExtractTexture(Moments, &MomentsHistory);
		Graph.ExtractTexture(DenoisedResult, &ShadowHistory);

		return DenoisedResult;
	}

	RenderGraphTextureRef RayTracedShadowsPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures)
	{
		// TODO: denoising with history

		static Vector3 LightDirection(1,1,1);
		static float LightRadius = 1;

		if (ImGui::Begin("Light"))
		{
			ImGui::SliderFloat3("Direction", (float*)&LightDirection, -1, 1);
			ImGui::SliderFloat("Radius", &LightRadius, 0, 5);
		}
		ImGui::End();

		TextureDesc2 Desc {
			.Usage = TextureUsage::Storage,
			.Width = (u32)View.OutputSize.X,
			.Height = (u32)View.OutputSize.Y,
			.Format = TextureFormat::R8,
		};
		RenderGraphTextureRef RTShadow = Graph.CreateTexture(Desc, "RayTracedShadow");

		// shouldn't be static, lifetime must be defined
		static SPtr<Texture2> History;

		// fallback?
		// RenderGraphTextureRef RTShadowHistory = Graph.RegisterExternalTexture(History, "RayTracedShadowHistory");

		{
			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies;
			Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTShadow, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			// Dependencies.Write(RTShadowHistory);

			Graph.AddPass("RayTraceShadow", RenderGraphPassType::Compute, Parameters, Dependencies, [RTShadow, Textures, View](RenderGraphContext& Context)
			{
				static RayTracingPipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					RayTracingPipelineDesc Desc;
					Desc.Name = "RayTracedShadowsPass";
					Desc.MaxRecursionDepth = 1;
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedShadowsPass.csd");

					Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
				}

				GPUCamera UpdatedCamera = GPUCamera(View.CameraCur);
				Context.Scene->Dirty = Context.Scene->MainCamera != UpdatedCamera; // TODO: move to the main rendering system
				Context.Scene->MainCamera = UpdatedCamera;

				auto ShadowsBufferSet = Context.GetDescriptorSet(Pipeline, 7);
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 0, 0, Context.Scene->TLAS);
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 1, 0, Context.GetRenderGraphTexture(RTShadow).get());
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get());
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 3, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get());

				if (History)
					Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 4, 0, History.get());

				// RTShadowParams Params { .Angle = Cvar_Angle.GetValue() };
				RTShadowParams Params {
					.Direction = LightDirection, .Angle = LightRadius,
					.Random = (rand() % 2000) / 2000.0f,
					.ValidHistory = History != nullptr && !Context.Scene->Dirty && false // TODO: remove history from here
				};

				Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Params), &Params);
				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindGPUScene(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 7, 1, &ShadowsBufferSet);

				Context.CommandBuffer->TraceRays(Pipeline, View.OutputSize.X, View.OutputSize.Y, 1);
			});
		}

		RTShadow = DenoiseRayTracedShadow(Graph, View, Textures, RTShadow);

		// creates/recreates image and copies it to be used in the next frame
		Graph.ExtractTexture(RTShadow, &History);

		return RTShadow;
	}

}
