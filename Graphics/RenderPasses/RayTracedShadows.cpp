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

	ConsoleVariable<bool> Cvar_Denoiser("r.RTShadows.Denoiser", "Controls whether shadow denoiser is active or not", true);

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedShadows);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedShadowsDenoise);

	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Shadows", "RenderGraphGPU", GpuCounterRayTracedShadows);
	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Shadows Denoise", "RenderGraphGPU", GpuCounterRayTracedShadowsDenoise);

	struct RTShadowParams
	{
		u32 Random;
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
		Matrix ViewToPrevViewMatrix; // camera-relative current-view -> previous-view (precision-safe)
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
	RenderGraphTextureRef DenoiseRayTracedShadow(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, RenderGraphTextureRef Shadow, RTShadowDenoiserHistory& History)
	{
		RENDER_GRAPH_SCOPED_MARKER(Graph, "Raytraced Shadows Denoise");

		iVector2 ShadowSize = View.RenderSize;

		// 8x4 tiles
		iVector2 TilesSize = iVector2((ShadowSize.X + 7) / 8, (ShadowSize.Y + 3) / 4);
		// 8x8 tiles
		iVector2 FilterTilesSize = iVector2((ShadowSize.X + 7) / 8, (ShadowSize.Y + 7) / 8);

		RenderGraphTextureRef RTShadowTiles;
		RenderGraphTextureRef Moments;
		RenderGraphTextureRef Metadata;
		RenderGraphTextureRef ReprojectionResult;
		RenderGraphTextureRef DenoisedResult;
		TextureDesc2 HistoryMomentsDesc { .Usage = TextureUsage::StorageSampled, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::R11G11B10F, };
		// Shadow history = filter PASS 0 output (mean, variance) in rg16f, matching FFX (SCRATCH1).
		TextureDesc2 HistoryShadowDesc { .Usage = TextureUsage::StorageSampled, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::RG16F, };
		const bool bMomentsHistoryInvalidated = Graph.CreateHistoryTexture(&History.Moments, HistoryMomentsDesc, "RayTracedShadows Moments History");
		const bool bShadowHistoryInvalidated = Graph.CreateHistoryTexture(&History.Shadow, HistoryShadowDesc, "RayTracedShadows Shadow History");
		const bool bFirstFrame = bMomentsHistoryInvalidated || bShadowHistoryInvalidated;

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

				iVector2 GroupCount = (TilesSize + 3) / 4; // prepare shader iterates over 4x4 tiles

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.CommandBuffer->Dispatch((u32)GroupCount.X, (u32)GroupCount.Y, 1);
			});
		}

		// Tile classification
		{
			TextureDesc2 MomentsDesc { .Usage = TextureUsage::StorageSampled, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::R11G11B10F, };
			TextureDesc2 MetadataDesc { .Usage = TextureUsage::Storage, .Width = (u32)FilterTilesSize.X, .Height = (u32)FilterTilesSize.Y, .Format = TextureFormat::R8UInt, };
			TextureDesc2 ReprojectionResultDesc { .Usage = TextureUsage::Storage, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::RG16F, };

			Moments = Graph.CreateTexture(MomentsDesc, "RayTracedShadowsMoments");
			Metadata = Graph.CreateTexture(MetadataDesc, "RayTracedShadowsMetadata");
			ReprojectionResult = Graph.CreateTexture(ReprojectionResultDesc, "RayTracedShadowsReprojectionResult");

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(RTShadowTiles, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.Velocity, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Dependencies.Write(Moments, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Metadata, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(ReprojectionResult, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("ShadowDenoiseTileClassification", RenderGraphPassType::Compute, Parameters, Dependencies,
			[RTShadowTiles, Textures, ShadowSize, FilterTilesSize, View, Moments, Metadata, ReprojectionResult, &History, bFirstFrame](RenderGraphContext& Context)
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
					Context.CommandBuffer->TransitionImageLayout(Textures.History.Depth, VK_IMAGE_LAYOUT_GENERAL);
					Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Textures.History.Depth, TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				}
				Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(Textures.Velocity).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

				Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, Context.GetRenderGraphTexture(ReprojectionResult).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 6, 0, Context.GetRenderGraphTexture(Moments).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 7, 0, History.Moments);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 8, 0, Context.GetRenderGraphTexture(Metadata).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 9, 0, History.Shadow, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 10, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());
				

				Matrix InvViewProjection = View.CameraCur.GetViewProjection().GetInverted();
				Matrix InvProjection = View.CameraCur.GetProjectionMatrix().GetInverted();

				// Camera-relative current-view -> previous-view transform.
				// M = R_prev * T(delta) * R_cur^-1, with delta = curPos - prevPos.
				// Jitter only affects the projection matrix (not view/pos), so CameraCur/Prev are safe here.
				// Keeping it camera-relative avoids the float32 precision loss of PrevVP*InvCurVP when the
				// camera is far from the world origin (the "fly around the level -> rejection" bug).
				Matrix Rcur = View.CameraCur.GetViewMatrix();
				Rcur.M[0][3] = Rcur.M[1][3] = Rcur.M[2][3] = 0.0f;
				Matrix Rprev = View.CameraPrev.GetViewMatrix();
				Rprev.M[0][3] = Rprev.M[1][3] = Rprev.M[2][3] = 0.0f;
				Matrix InvRcur = Rcur.GetInverted();

				Vector3 camDelta = View.CameraCur.Pos - View.CameraPrev.Pos;
				Matrix Tdelta; // identity
				Tdelta.M[0][3] = camDelta.X;
				Tdelta.M[1][3] = camDelta.Y;
				Tdelta.M[2][3] = camDelta.Z;

				Matrix ViewToPrevView = Rprev * Tdelta * InvRcur;

				RTShadowDenoiserTileClassificationParams Params {
					.InvViewProjectionMatrix = InvViewProjection,
					.InvProjectionMatrix = InvProjection,
					.ViewToPrevViewMatrix = ViewToPrevView,
					.CameraPosition = View.CameraCur.Pos,
					.BufferDimensions = ShadowSize,
					.PackedBufferDimensions = FilterTilesSize,
					.FirstFrame = bFirstFrame,
				};

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.CommandBuffer->Dispatch((u32)FilterTilesSize.X, (u32)FilterTilesSize.Y, 1);
			});			
		}

		// Filter
		RenderGraphTextureRef ShadowHistoryFeedback; // filter pass 0 output -> next frame's history (FFX SCRATCH1)
		{
			RENDER_GRAPH_SCOPED_MARKER(Graph, "Raytraced Shadows Denoise Filter");

			TextureDesc2 ResultDesc { .Usage = TextureUsage::StorageSampled, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::R8, };
			DenoisedResult = Graph.CreateTexture(ResultDesc, "ShadowDenoised");

			TextureDesc2 TmpHistoryDesc { .Usage = TextureUsage::StorageSampled, .Width = (u32)ShadowSize.X, .Height = (u32)ShadowSize.Y, .Format = TextureFormat::RG16F, };
			RenderGraphTextureRef TmpHistory0 = Graph.CreateTexture(TmpHistoryDesc, "ShadowDenoiserFilterTmpHistory0");
			RenderGraphTextureRef TmpHistory1 = Graph.CreateTexture(TmpHistoryDesc, "ShadowDenoiserFilterTmpHistory1");
			RenderGraphTextureRef UnusedFinalOutput = Graph.CreateTexture(ResultDesc, "ShadowDenoiserUnusedFinalOutput");
			RenderGraphTextureRef UnusedHistoryOutput = Graph.CreateTexture(TmpHistoryDesc, "ShadowDenoiserUnusedHistoryOutput");

			// FFX feeds back filter PASS 0's output as the temporal history (not the final pass-2 result).
			ShadowHistoryFeedback = TmpHistory0;

			RenderGraphTextureRef CurrentFilterInput = ReprojectionResult;
			const int NumPasses = 3;

			for (int i = 0; i < NumPasses; i++)
			{
				const bool bFinalPass = i == NumPasses - 1;
				const RenderGraphTextureRef FilterOutput = (i == 0) ? TmpHistory0 : (i == 1) ? TmpHistory1 : DenoisedResult;
				const RenderGraphTextureRef FinalOutputBinding = bFinalPass ? DenoisedResult : UnusedFinalOutput;
				const RenderGraphTextureRef HistoryOutputBinding = bFinalPass ? UnusedHistoryOutput : FilterOutput;
				RenderPassParameters Parameters;

				RenderPassDependencies Dependencies(Graph.Allocator);
				Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				Dependencies.Read(CurrentFilterInput, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				Dependencies.Read(Metadata, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				Dependencies.Write(FinalOutputBinding, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				Dependencies.Write(HistoryOutputBinding, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

				Graph.AddPass("ShadowDenoiseFilter", RenderGraphPassType::Compute, Parameters, Dependencies,
				[i, View, Textures, CurrentFilterInput, FinalOutputBinding, HistoryOutputBinding, Metadata, ShadowSize, FilterTilesSize](RenderGraphContext& Context)
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
					Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(FinalOutputBinding).get());
					Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, Context.GetRenderGraphTexture(HistoryOutputBinding).get());

					Context.CommandBuffer->BindComputePipeline(Pipelines[i]);
					Context.CommandBuffer->PushConstantsCompute(Pipelines[i], ShaderType::Compute, 0, sizeof(Params), &Params);
					Context.CommandBuffer->BindDescriptorSetsCompute(Pipelines[i], 0, 1, &DescriptorSet);
					Context.CommandBuffer->Dispatch((u32)FilterTilesSize.X, (u32)FilterTilesSize.Y, 1);
				});

				CurrentFilterInput = FilterOutput;
			}
		}

		Graph.ExtractTexture(Moments, &History.Moments);
		// Persist filter pass 0's output (lightly filtered mean+variance), NOT the final pass-2 result.
		// This matches FFX: tile classification reads this as shadow_previous next frame.
		Graph.ExtractTexture(ShadowHistoryFeedback, &History.Shadow);

		return DenoisedResult;
	}

	void RayTracedShadowsPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& DeferredContext)
	{
		RENDER_GRAPH_SCOPED_MARKER(Graph, "RayTracedShadows");

		TextureDesc2 Desc {
			.Usage = TextureUsage::Storage,
			.Width = (u32)View.RenderSize.X,
			.Height = (u32)View.RenderSize.Y,
			.Format = TextureFormat::R8,
		};

		for (int i = 0; i < Graph.Scene->Lights.Size(); i++)
		{
			GPULight& Light = Graph.Scene->Lights[i];

			RenderGraphTextureRef RTShadow = Graph.CreateTexture(Desc, "RayTracedShadow");

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTShadow, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass("RayTraceShadow", RenderGraphPassType::Compute, Parameters, Dependencies, [RTShadow, Textures, View, i](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedShadows, Context);

				static RayTracingPipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					RayTracingPipelineDesc Desc;
					Desc.Name = "RayTracedShadowsPass";
					Desc.MaxRecursionDepth = 1;
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedShadows/RayTraceShadows.csd");

					Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
				}

				GPUCamera UpdatedCamera = GPUCamera(View.CameraCur);
				Context.Scene->Dirty = Context.Scene->MainCamera != UpdatedCamera; // TODO: move to the main rendering system
				Context.Scene->MainCamera = UpdatedCamera;

				auto ShadowsBufferSet = Context.GetDescriptorSet(Pipeline, 2);
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 0, 0, Context.Scene->TLAS);
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 2, 0, Context.GetRenderGraphTexture(RTShadow).get());

				RTShadowParams Params {
					.Random = (u32)(rand() % 2000),
					.LightId = (u32)i
				};

				Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Params), &Params);
				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindGPUScene(Pipeline, false);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &ShadowsBufferSet);

				Context.CommandBuffer->TraceRays(Pipeline, View.RenderSize.X, View.RenderSize.Y, 1);
			});

			if (Cvar_Denoiser.GetValue() && Light.NeedsRTShadowDenoiser())
			{
				RTShadow = DenoiseRayTracedShadow(Graph, View, Textures, RTShadow, Light.RTShadow);
			}

			// TODO: improve that mechanism
			GPULightRenderInfo LightInfo {
				.RTShadow = RTShadow
			};
			DeferredContext.LightRenderInfos.push_back(LightInfo);
		}
	}

}
