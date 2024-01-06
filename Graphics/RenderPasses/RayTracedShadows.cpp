#include "Common/Image/Image.h"
#include "Core/CVar.h"
#include "Graphics/Core/Pipelines.h"
#include "Graphics/Core/Types.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/Vulkan/DeviceVulkan.h"
#include "RenderPasses.h"
#include "imgui.h"

namespace Columbus
{

	ConsoleVariable<bool> Cvar_Denoiser("r.RTShadows.Denoiser", "Controls whether shadow denoiser is active or not", false);

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedShadows);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedShadowsDenoise);

	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Shadows", "RenderGraphGPU", GpuCounterRayTracedShadows);
	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Shadows Denoise", "RenderGraphGPU", GpuCounterRayTracedShadowsDenoise);

	struct RTShadowParams
	{
		float Random;
		u32 LightId;
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

	// implements AMD FidelityFX Shadow Denoiser
	RenderGraphTextureRef DenoiseRayTracedShadow(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, RenderGraphTextureRef Shadow)
	{
		RENDER_GRAPH_SCOPED_MARKER(Graph, "ShadowsDenoise");

		iVector2 ShadowSize = View.OutputSize;
		// 8x4 tiles
		iVector2 TilesSize = iVector2((View.OutputSize.X + 7) / 8, (View.OutputSize.Y + 3) / 4);
		// 8x8 tiles
		iVector2 FilterTilesSize = iVector2((View.OutputSize.X + 7) / 8, (View.OutputSize.Y + 7) / 8);

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

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Shadow, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(RTShadowTiles, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("ShadowDenoisePrepare", RenderGraphPassType::Compute, Parameters, Dependencies, [TilesSize, Shadow, RTShadowTiles, ShadowSize](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedShadowsDenoise, Context);

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

				iVector2 GroupCount = TilesSize / 4; // prepare shader iterates over 4x4 tiles

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.CommandBuffer->Dispatch((u32)GroupCount.X, (u32)GroupCount.Y, 1);
			});
		}

		// Tile classification
		{
			TextureDesc2 MomentsDesc { .Usage = TextureUsage::Storage, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::R11G11B10F, };
			TextureDesc2 MetadataDesc { .Usage = TextureUsage::Storage, .Width = (u32)FilterTilesSize.X, .Height = (u32)FilterTilesSize.Y, .Format = TextureFormat::R8UInt, };
			TextureDesc2 ReprojectionResultDesc { .Usage = TextureUsage::Storage, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::RG16F, };
			// TODO: move history textures to context

			Moments = Graph.CreateTexture(MomentsDesc, "RayTracedShadowsMoments");
			Metadata = Graph.CreateTexture(MetadataDesc, "RayTracedShadowsMetadata");
			ReprojectionResult = Graph.CreateTexture(ReprojectionResultDesc, "RayTracedShadowsReprojectionResult");

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(RTShadowTiles, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.Velocity, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Dependencies.Write(Moments, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Metadata, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(ReprojectionResult, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("ShadowDenoiseTileClassification", RenderGraphPassType::Compute, Parameters, Dependencies,
			[RTShadowTiles, Textures, ShadowSize, FilterTilesSize, View, Moments, Metadata, ReprojectionResult](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedShadowsDenoise, Context);

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
					.ReprojectionMatrix = View.CameraPrev.GetViewProjection() * InvViewProjection,
					.CameraPosition = View.CameraCur.Pos,
					.BufferDimensions = ShadowSize,
					.PackedBufferDimensions = FilterTilesSize,
					.FirstFrame = ShadowHistory == nullptr,
				};

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.CommandBuffer->Dispatch((u32)FilterTilesSize.X, (u32)FilterTilesSize.Y, 1);
			});			
		}

		// Filter
		{
			RENDER_GRAPH_SCOPED_MARKER(Graph, "FilterDenoise");

			TextureDesc2 ResultDesc { .Usage = TextureUsage::Storage, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::R8, };
			DenoisedResult = Graph.CreateTexture(ResultDesc, "ShadowDenoised");

			TextureDesc2 TmpHistoryDesc { .Usage = TextureUsage::Storage, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::RG16F, };
			RenderGraphTextureRef TmpHistory = Graph.CreateTexture(TmpHistoryDesc, "ShadowDenoiserFilterTmpHistory");

			RenderGraphTextureRef CurrentFilterInput = ReprojectionResult;
			const int NumPasses = 3;

			for (int i = 0; i < NumPasses; i++)
			{
				RenderPassParameters Parameters;

				RenderPassDependencies Dependencies(Graph.Allocator);
				Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				Dependencies.Read(CurrentFilterInput, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				Dependencies.Read(Metadata, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				Dependencies.Write(DenoisedResult, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				if (i == 0)
					Dependencies.Write(TmpHistory, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

				Graph.AddPass("ShadowDenoiseFilter", RenderGraphPassType::Compute, Parameters, Dependencies,
				[i, View, Textures, CurrentFilterInput, TmpHistory, Metadata, DenoisedResult, ShadowSize, FilterTilesSize](RenderGraphContext& Context)
				{
					RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedShadowsDenoise, Context);

					static ComputePipeline* Pipelines[NumPasses] {nullptr};
					if (Pipelines[i] == nullptr)
					{
						ComputePipelineDesc Desc;
						Desc.Name = "DenoiserShadowsFilter";
						Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedShadows/DenoiserShadowsFilter.csd");

						Pipelines[i] = Context.Device->CreateComputePipeline(Desc);
					}

					const u32 StepSizes[] = { 1, 2, 4 };

					RTShadowDenoiserFilterParams Params {
						.InvProjectionMatrix = View.CameraCur.GetProjectionMatrix().GetInverted(),
						.BufferDimensions = ShadowSize,
						.PassIndex = (u32)i,
						.StepSize = StepSizes[i],
					};

					auto DescriptorSet = Context.GetDescriptorSet(Pipelines[i], 0);
					Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
					Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
					Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(CurrentFilterInput).get());
					Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.GetRenderGraphTexture(Metadata).get());
					Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(DenoisedResult).get());
					Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, Context.GetRenderGraphTexture(TmpHistory).get());

					Context.CommandBuffer->BindComputePipeline(Pipelines[i]);
					Context.CommandBuffer->PushConstantsCompute(Pipelines[i], ShaderType::Compute, 0, sizeof(Params), &Params);
					Context.CommandBuffer->BindDescriptorSetsCompute(Pipelines[i], 0, 1, &DescriptorSet);
					Context.CommandBuffer->Dispatch((u32)FilterTilesSize.X, (u32)FilterTilesSize.Y, 1);
				});

				CurrentFilterInput = TmpHistory;
			}
		}

		Graph.ExtractTexture(Moments, &MomentsHistory);
		Graph.ExtractTexture(DenoisedResult, &ShadowHistory);

		return DenoisedResult;
	}

	void RayTracedShadowsPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& DeferredContext)
	{
		RENDER_GRAPH_SCOPED_MARKER(Graph, "RayTracedShadows");

		TextureDesc2 Desc {
			.Usage = TextureUsage::Storage,
			.Width = (u32)View.OutputSize.X,
			.Height = (u32)View.OutputSize.Y,
			.Format = TextureFormat::R8,
		};

		// TODO:
		static VkDescriptorSet RayDescriptorSets[MaxFramesInFlight][64]{NULL};

		for (int i = 0; i < Graph.Scene->Lights.size(); i++)
		{
			GPULight& Light = Graph.Scene->Lights[i];

			RenderGraphTextureRef RTShadow = Graph.CreateTexture(Desc, "RayTracedShadow");

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTShadow, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			char PassName[256]{0};
			snprintf(PassName, 256, "RayTraceShadow %i (%s)", i, LightTypeToString(Light.Type));

			Graph.AddPass(PassName, RenderGraphPassType::Compute, Parameters, Dependencies, [RTShadow, Textures, View, i](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedShadows, Context);

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

				if (RayDescriptorSets[Context.RenderData.CurrentPerFrameData][i] == NULL)
				{
					RayDescriptorSets[Context.RenderData.CurrentPerFrameData][i] = Context.Device->CreateDescriptorSet(Pipeline, 2);
				}

				auto ShadowsBufferSet = RayDescriptorSets[Context.RenderData.CurrentPerFrameData][i];
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 0, 0, Context.Scene->TLAS);
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 1, 0, Context.GetRenderGraphTexture(RTShadow).get());
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get());
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 3, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get());

				RTShadowParams Params {
					.Random = (rand() % 2000) / 2000.0f,
					.LightId = (u32)i
				};

				Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Params), &Params);
				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindGPUScene(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &ShadowsBufferSet);

				Context.CommandBuffer->TraceRays(Pipeline, View.OutputSize.X, View.OutputSize.Y, 1);
			});

			// TODO: improve that mechanism
			GPULightRenderInfo LightInfo {
				.RTShadow = RTShadow
			};
			DeferredContext.LightRenderInfos.push_back(LightInfo);

			if (Cvar_Denoiser.GetValue())
			{
				RTShadow = DenoiseRayTracedShadow(Graph, View, Textures, RTShadow);
			}
		}
	}

}
