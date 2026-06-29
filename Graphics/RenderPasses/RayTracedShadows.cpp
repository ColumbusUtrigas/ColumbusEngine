#include "Common/Image/Image.h"
#include "Core/CVar.h"
#include "Graphics/Core/Types.h"
#include "Graphics/ShaderCache.h"
#include "RenderPasses.h"

namespace Columbus
{

	ConsoleVariable<bool> Cvar_Denoiser("r.RTShadows.Denoiser", "Controls whether shadow denoiser is active or not", true);

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedShadows);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedShadowsDenoise);

	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Shadows", "RenderGraphGPU", GpuCounterRayTracedShadows);
	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Shadows Denoise", "RenderGraphGPU", GpuCounterRayTracedShadowsDenoise);

	struct RTShadowParams
	{
		u32 Random = 0;
		u32 LightId = 0;
	};

	struct RTShadowDenoiserPrepareParams
	{
		iVector2 BufferDimensions{};
		iVector2 PackedBufferDimensions{};
	};

	struct RTShadowDenoiserTileClassificationParams
	{
		Matrix InvViewProjectionMatrix = Matrix(1.0f);
		Matrix InvProjectionMatrix = Matrix(1.0f);
		Matrix ViewToPrevViewMatrix = Matrix(1.0f); // camera-relative current-view -> previous-view (precision-safe)
		Vector3 CameraPosition{};
		float _pad = 0.0f;
		iVector2 BufferDimensions{};
		iVector2 PackedBufferDimensions{};
		int FirstFrame = 0;
	};

	struct RTShadowDenoiserFilterParams
	{
		Matrix InvProjectionMatrix = Matrix(1.0f);
		iVector2 BufferDimensions{};
		u32 PassIndex = 0;
		u32 StepSize = 1;
	};

	struct RTShadowTraceShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedShadows/RayTraceShadows.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct PipelinePermutation
		{
			u32 MaxRecursionDepth = 1;
		};

		static RayTracingPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			RayTracingPipelineDesc Desc {};
			Desc.Name = "RayTracedShadowsPass";
			Desc.MaxRecursionDepth = Permutation.MaxRecursionDepth;
			return Desc;
		}

		struct Parameters
		{
			ShaderGPUScene Scene;
			ShaderAccelerationStructure AccelerationStructure;
			ShaderSampledTexture InputDepth { TextureBindingFlags::AspectDepth };
			ShaderStorageTexture OutputShadow;
			ShaderPushConstants<RTShadowParams> Constants { {}, ShaderType::Raygen };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.AccelerationStructure, 2, 0);
			Binder.Bind(Params.InputDepth, 2, 1);
			Binder.Bind(Params.OutputShadow, 2, 2);
			Binder.Bind(Params.Constants);
		}
	};

	struct RTShadowDenoiserPrepareShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedShadows/DenoiserShadowsPrepare.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderReadStorageTexture InputBuffer;
			ShaderStorageTexture PackedOutputBuffer;
			ShaderPushConstants<RTShadowDenoiserPrepareParams> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.InputBuffer, 0, 0);
			Binder.Bind(Params.PackedOutputBuffer, 0, 1);
			Binder.Bind(Params.Constants);
		}
	};

	struct RTShadowDenoiserTileClassificationShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedShadows/DenoiserShadowsTileClassification.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderReadStorageTexture PackedTilesBuffer;
			ShaderSampledTexture Normals;
			ShaderSampledTexture Depth { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture DepthHistory { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture Velocity;
			ShaderStorageTexture ReprojectionResult;
			ShaderStorageTexture Moments;
			ShaderReadStorageTexture MomentsHistory;
			ShaderStorageTexture Metadata;
			ShaderSampledTexture History;
			ShaderStaticSampler HistorySampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderPushConstants<RTShadowDenoiserTileClassificationParams> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.PackedTilesBuffer, 0, 0);
			Binder.Bind(Params.Normals, 0, 1);
			Binder.Bind(Params.Depth, 0, 2);
			Binder.Bind(Params.DepthHistory, 0, 3);
			Binder.Bind(Params.Velocity, 0, 4);
			Binder.Bind(Params.ReprojectionResult, 0, 5);
			Binder.Bind(Params.Moments, 0, 6);
			Binder.Bind(Params.MomentsHistory, 0, 7);
			Binder.Bind(Params.Metadata, 0, 8);
			Binder.Bind(Params.History, 0, 9);
			Binder.Bind(Params.HistorySampler, 0, 10);
			Binder.Bind(Params.Constants);
		}
	};

	struct RTShadowDenoiserFilterShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedShadows/DenoiserShadowsFilter.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderSampledTexture Normals;
			ShaderSampledTexture Depth { TextureBindingFlags::AspectDepth };
			ShaderReadStorageTexture Input;
			ShaderReadStorageTexture Metadata;
			ShaderStorageTexture Output;
			ShaderStorageTexture History;
			ShaderPushConstants<RTShadowDenoiserFilterParams> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Normals, 0, 0);
			Binder.Bind(Params.Depth, 0, 1);
			Binder.Bind(Params.Input, 0, 2);
			Binder.Bind(Params.Metadata, 0, 3);
			Binder.Bind(Params.Output, 0, 4);
			Binder.Bind(Params.History, 0, 5);
			Binder.Bind(Params.Constants);
		}
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

			RTShadowDenoiserPrepareShader::Parameters PrepareParams;
			PrepareParams.InputBuffer = Shadow;
			PrepareParams.PackedOutputBuffer = RTShadowTiles;
			PrepareParams.Constants.Value = {
				.BufferDimensions = ShadowSize,
				.PackedBufferDimensions = TilesSize,
			};

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<RTShadowDenoiserPrepareShader>(PrepareParams);

			Graph.AddPass("ShadowDenoisePrepare", RenderGraphPassType::Compute, Parameters, Dependencies, [TilesSize, PrepareParams](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedShadowsDenoise, Context);

				iVector2 GroupCount = (TilesSize + 3) / 4; // prepare shader iterates over 4x4 tiles

				ComputePipeline* Pipeline = GetComputePipeline<RTShadowDenoiserPrepareShader>(Context, RTShadowDenoiserPrepareShader::Permutation {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.BindComputeParameters<RTShadowDenoiserPrepareShader>(Pipeline, PrepareParams);
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

			RTShadowDenoiserTileClassificationShader::Parameters TileParams;
			TileParams.PackedTilesBuffer = RTShadowTiles;
			TileParams.Normals = Textures.GBufferNormal;
			TileParams.Depth = Textures.GBufferDS;
			if (Textures.History.Depth)
			{
				TileParams.DepthHistory = Textures.History.Depth;
			}
			else
			{
				TileParams.DepthHistory = Textures.GBufferDS;
			}
			TileParams.Velocity = Textures.Velocity;
			TileParams.ReprojectionResult = ReprojectionResult;
			TileParams.Moments = Moments;
			TileParams.MomentsHistory = History.Moments;
			TileParams.Metadata = Metadata;
			TileParams.History = History.Shadow;
			TileParams.Constants.Value = {
				.InvViewProjectionMatrix = InvViewProjection,
				.InvProjectionMatrix = InvProjection,
				.ViewToPrevViewMatrix = ViewToPrevView,
				.CameraPosition = View.CameraCur.Pos,
				.BufferDimensions = ShadowSize,
				.PackedBufferDimensions = FilterTilesSize,
				.FirstFrame = bFirstFrame,
			};

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<RTShadowDenoiserTileClassificationShader>(TileParams);

			Graph.AddPass("ShadowDenoiseTileClassification", RenderGraphPassType::Compute, Parameters, Dependencies,
			[TileParams, Textures, FilterTilesSize](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedShadowsDenoise, Context);

				if (Textures.History.Depth)
				{
					Context.CommandBuffer->TransitionImageLayout(Textures.History.Depth, VK_IMAGE_LAYOUT_GENERAL);
				}

				ComputePipeline* Pipeline = GetComputePipeline<RTShadowDenoiserTileClassificationShader>(Context, RTShadowDenoiserTileClassificationShader::Permutation {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.BindComputeParameters<RTShadowDenoiserTileClassificationShader>(Pipeline, TileParams);
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

				const u32 StepSizes[] = { 1, 2, 4 };

				RTShadowDenoiserFilterShader::Parameters FilterParams;
				FilterParams.Normals = Textures.GBufferNormal;
				FilterParams.Depth = Textures.GBufferDS;
				FilterParams.Input = CurrentFilterInput;
				FilterParams.Metadata = Metadata;
				FilterParams.Output = FinalOutputBinding;
				FilterParams.History = HistoryOutputBinding;
				FilterParams.Constants.Value = {
					.InvProjectionMatrix = View.CameraCur.GetProjectionMatrix().GetInverted(),
					.BufferDimensions = ShadowSize,
					.PassIndex = (u32)i,
					.StepSize = StepSizes[i],
				};

				RenderPassParameters Parameters;

				RenderPassDependencies Dependencies(Graph.Allocator);
				Dependencies.Bind<RTShadowDenoiserFilterShader>(FilterParams);

				Graph.AddPass("ShadowDenoiseFilter", RenderGraphPassType::Compute, Parameters, Dependencies,
				[FilterParams, FilterTilesSize](RenderGraphContext& Context)
				{
					RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedShadowsDenoise, Context);

					ComputePipeline* Pipeline = GetComputePipeline<RTShadowDenoiserFilterShader>(Context, RTShadowDenoiserFilterShader::Permutation {});
					Context.CommandBuffer->BindComputePipeline(Pipeline);
					Context.BindComputeParameters<RTShadowDenoiserFilterShader>(Pipeline, FilterParams);
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

			RTShadowTraceShader::Parameters TraceParams;
			TraceParams.Scene.UseCombinedSampler = false;
			TraceParams.InputDepth = Textures.GBufferDS;
			TraceParams.OutputShadow = RTShadow;
			TraceParams.Constants.Value = {
				.Random = 0,
				.LightId = (u32)i,
			};

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<RTShadowTraceShader>(TraceParams);

			Graph.AddPass("RayTraceShadow", RenderGraphPassType::Compute, Parameters, Dependencies, [TraceParams, View](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedShadows, Context);

				GPUCamera UpdatedCamera = GPUCamera(View.CameraCur);
				Context.Scene->Dirty = Context.Scene->MainCamera != UpdatedCamera; // TODO: move to the main rendering system
				Context.Scene->MainCamera = UpdatedCamera;

				RTShadowTraceShader::Parameters Parameters = TraceParams;
				Parameters.AccelerationStructure = Context.Scene->TLAS;
				Parameters.Constants.Value.Random = (u32)(rand() % 2000);

				RayTracingPipeline* Pipeline = GetRayTracingPipeline<RTShadowTraceShader>(Context, RTShadowTraceShader::Permutation {}, RTShadowTraceShader::PipelinePermutation {});
				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindRayTracingParameters<RTShadowTraceShader>(Pipeline, Parameters);
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
