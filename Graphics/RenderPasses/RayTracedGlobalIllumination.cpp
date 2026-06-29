#include "RenderPasses.h"
#include "RayTracingIrradianceVolumes.h"
#include "Core/CVar.h"
#include "Graphics/ShaderCache.h"

namespace Columbus
{

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedGI);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedGIPrepare);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedGIDenoise);

	IMPLEMENT_GPU_PROFILING_COUNTER("RTGI", "RenderGraphGPU", GpuCounterRayTracedGI);
	IMPLEMENT_GPU_PROFILING_COUNTER("RTGI Prepare", "RenderGraphGPU", GpuCounterRayTracedGIPrepare);
	IMPLEMENT_GPU_PROFILING_COUNTER("RTGI Denoise", "RenderGraphGPU", GpuCounterRayTracedGIDenoise);

	void RTGIDenoiserHistory::Destroy(SPtr<DeviceVulkan> Device)
	{
		Device->DestroyTexture(Radiance);
		Device->DestroyTexture(AverageRadiance);
		Device->DestroyTexture(Variance);
		Device->DestroyTexture(SampleCount);
		Radiance = nullptr;
		AverageRadiance = nullptr;
		Variance = nullptr;
		SampleCount = nullptr;
	}

	struct SimpleDenoiseSpatialParameters
	{
		iVector2 Size{};
		int StepSize = 0;
		int MaxSamples = 0;
		int DominationNumber = 0;
	};

	struct SimpleDenoiserTemporalParameters
	{
		Matrix   ProjectionInv = Matrix(1.0f);
		Matrix   ViewProjectionInv = Matrix(1.0f);
		Matrix   PrevViewProjection = Matrix(1.0f);
		iVector2 Size{};
		int MaxSamples = 0;
	};

	struct DownsampleGBufferParameters
	{
		iVector2 Size{};
		iVector2 SourceSize{};
		int DownsampleFactor = 1;
	};

	struct UpsampleRTGIParameters
	{
		iVector2 Size{};
		iVector2 InputSize{};
		int DownsampleFactor = 1;
	};

	struct RTGI_Parameters
	{
		Vector3 CameraPosition{};
		u32     Random = 0;
		float   DiffuseBoost = 1.0f;
		u32     UseRadianceCache = 0;
		float   UpscaleFactor = 1.0f;
	};

	struct DownsampledTextures
	{
		RenderGraphTextureRef Depth;
		RenderGraphTextureRef Normal;
		RenderGraphTextureRef DepthHistory;
		RenderGraphTextureRef Velocity;
		RenderGraphTextureRef SourcePixel;
	};

	struct RTGIDownsampleShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedGlobalIllumination/Downsample.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderSampledTexture Depth { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture Normals;
			ShaderSampledTexture DepthHistory { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture Velocity;
			ShaderStorageTexture OutputDepth;
			ShaderStorageTexture OutputNormals;
			ShaderStorageTexture OutputDepthHistory;
			ShaderStorageTexture OutputVelocity;
			ShaderStorageTexture OutputSourcePixel;
			ShaderPushConstants<DownsampleGBufferParameters> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Depth, 0, 0);
			Binder.Bind(Params.Normals, 0, 1);
			Binder.Bind(Params.DepthHistory, 0, 2);
			Binder.Bind(Params.Velocity, 0, 3);
			Binder.Bind(Params.OutputDepth, 0, 4);
			Binder.Bind(Params.OutputNormals, 0, 5);
			Binder.Bind(Params.OutputDepthHistory, 0, 6);
			Binder.Bind(Params.OutputVelocity, 0, 7);
			Binder.Bind(Params.OutputSourcePixel, 0, 8);
			Binder.Bind(Params.Constants);
		}
	};

	struct RTGISpatialShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedGlobalIllumination/Spatial.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderSampledTexture Depth;
			ShaderSampledTexture Normals;
			ShaderSampledTexture SampleCount;
			ShaderSampledTexture Input;
			ShaderStorageTexture Output;
			ShaderPushConstants<SimpleDenoiseSpatialParameters> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Depth, 0, 0);
			Binder.Bind(Params.Normals, 0, 1);
			Binder.Bind(Params.SampleCount, 0, 2);
			Binder.Bind(Params.Input, 0, 3);
			Binder.Bind(Params.Output, 0, 4);
			Binder.Bind(Params.Constants);
		}
	};

	struct RTGITemporalShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedGlobalIllumination/Temporal.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderSampledTexture Velocity;
			ShaderSampledTexture Input;
			ShaderSampledTexture History;
			ShaderSampledTexture Depth;
			ShaderSampledTexture DepthHistory;
			ShaderStorageTexture SampleCount;
			ShaderStorageTexture Output;
			ShaderStaticSampler Sampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderPushConstants<SimpleDenoiserTemporalParameters> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Velocity, 0, 0);
			Binder.Bind(Params.Input, 0, 1);
			Binder.Bind(Params.History, 0, 2);
			Binder.Bind(Params.Depth, 0, 3);
			Binder.Bind(Params.DepthHistory, 0, 4);
			Binder.Bind(Params.SampleCount, 0, 5);
			Binder.Bind(Params.Output, 0, 6);
			Binder.Bind(Params.Sampler, 0, 7);
			Binder.Bind(Params.Constants);
		}
	};

	struct RTGIUpsampleShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedGlobalIllumination/Upsample.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderSampledTexture Input;
			ShaderSampledTexture Depth { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture Normals;
			ShaderSampledTexture LowResDepth;
			ShaderStorageTexture Output;
			ShaderPushConstants<UpsampleRTGIParameters> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Input, 0, 0);
			Binder.Bind(Params.Depth, 0, 1);
			Binder.Bind(Params.Normals, 0, 2);
			Binder.Bind(Params.LowResDepth, 0, 3);
			Binder.Bind(Params.Output, 0, 4);
			Binder.Bind(Params.Constants);
		}
	};

	struct RayTracedGIShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedGlobalIllumination/RayTracedGI.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct PipelinePermutation
		{
			u32 MaxRecursionDepth = 1;
		};

		static RayTracingPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			RayTracingPipelineDesc Desc {};
			Desc.Name = "RayTracedGI";
			Desc.MaxRecursionDepth = Permutation.MaxRecursionDepth;
			return Desc;
		}

		struct Parameters
		{
			ShaderGPUScene Scene;
			ShaderAccelerationStructure AccelerationStructure;
			ShaderStorageTexture Output;
			ShaderSampledTexture GBufferNormals;
			ShaderSampledTexture GBufferDepth;
			ShaderSampledTexture GBufferSourcePixel;
			ShaderConstants<RayTracingIrradianceVolumes::Constants> IrradianceVolumeConstants;
			std::array<ShaderReadBuffer, RayTracingIrradianceVolumes::MaxVolumes> IrradianceProbeBuffers;
			ShaderPushConstants<RTGI_Parameters> Constants { {}, ShaderType::Raygen };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.AccelerationStructure, 2, 0);
			Binder.Bind(Params.Output, 2, 1);
			Binder.Bind(Params.GBufferNormals, 2, 2);
			Binder.Bind(Params.GBufferDepth, 2, 3);
			Binder.Bind(Params.GBufferSourcePixel, 2, 4);
			Binder.Bind(Params.IrradianceVolumeConstants, 2, 9);
			ShaderArray<ShaderReadBuffer> IrradianceProbeBuffers;
			IrradianceProbeBuffers.Set(Params.IrradianceProbeBuffers.data(), RayTracingIrradianceVolumes::MaxVolumes);
			Binder.Bind(IrradianceProbeBuffers, 2, 10);
			Binder.Bind(Params.Constants);
		}
	};

	static DownsampledTextures DownsampleGBuffer(RenderGraph& Graph, SceneTextures& Textures, int DownsampleFactor)
	{
		const iVector2 SourceSize = Graph.GetTextureSize2D(Textures.GBufferNormal);
		const iVector2 Size = (SourceSize + DownsampleFactor - 1) / DownsampleFactor;

		DownsampledTextures Result;

		if (DownsampleFactor == 1)
		{
			// TODO:
		}

		TextureDesc2 CommonDesc;
		CommonDesc.Width  = (u32)Size.X;
		CommonDesc.Height = (u32)Size.Y;
		CommonDesc.Usage  = TextureUsage::StorageSampled;

		TextureDesc2 DepthDesc     = CommonDesc;
		TextureDesc2 NormalDesc    = CommonDesc;
		TextureDesc2 DepthHistDesc = CommonDesc;
		TextureDesc2 VelocityDesc  = CommonDesc;
		TextureDesc2 SourcePixelDesc = CommonDesc;

		DepthDesc.Format     = TextureFormat::R32F;
		NormalDesc.Format    = Graph.GetTextureDesc(Textures.GBufferNormal).Format;
		DepthHistDesc.Format = TextureFormat::R32F;
		VelocityDesc.Format  = Graph.GetTextureDesc(Textures.Velocity).Format;
		SourcePixelDesc.Format = TextureFormat::RGBA32F;

		Result.Depth        = Graph.CreateTexture(DepthDesc,    "Downsampled Depth");
		Result.Normal       = Graph.CreateTexture(NormalDesc,   "Downsampled Normals");
		Result.DepthHistory = Graph.CreateTexture(DepthHistDesc, "Downsampled DepthHistory");
		Result.Velocity     = Graph.CreateTexture(VelocityDesc, "Downsampled Velocity");
		Result.SourcePixel  = Graph.CreateTexture(SourcePixelDesc, "Downsampled SourcePixel");

		RTGIDownsampleShader::Parameters DownsampleParams;
		DownsampleParams.Depth = Textures.GBufferDS;
		DownsampleParams.Normals = Textures.GBufferNormal;
		DownsampleParams.DepthHistory = Textures.History.Depth;
		DownsampleParams.Velocity = Textures.Velocity;
		DownsampleParams.OutputDepth = Result.Depth;
		DownsampleParams.OutputNormals = Result.Normal;
		DownsampleParams.OutputDepthHistory = Result.DepthHistory;
		DownsampleParams.OutputVelocity = Result.Velocity;
		DownsampleParams.OutputSourcePixel = Result.SourcePixel;
		DownsampleParams.Constants.Value = {
			.Size = Size,
			.SourceSize = SourceSize,
			.DownsampleFactor = DownsampleFactor,
		};

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RTGIDownsampleShader>(DownsampleParams);

		Graph.AddPass("Downsample GBuffer", RenderGraphPassType::Compute, Parameters, Dependencies, [DownsampleParams, Size](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIPrepare, Context);

			ComputePipeline* Pipeline = GetComputePipeline<RTGIDownsampleShader>(Context, RTGIDownsampleShader::Permutation {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<RTGIDownsampleShader>(Pipeline, DownsampleParams);
			Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { Size, 1 });
		});

		return Result;
	}

	template <int Perm>
	static void SimpleDenoiseSpatial(RenderGraph& Graph, SceneTextures& Textures, DownsampledTextures DownTextures, iVector2 Size, int StepSize, RenderGraphTextureRef Radiance1, RenderGraphTextureRef Radiance2, int MaxSamples, int DominationSampleNumber)
	{
		RTGISpatialShader::Parameters SpatialParams;
		SpatialParams.Depth = DownTextures.Depth;
		SpatialParams.Normals = DownTextures.Normal;
		SpatialParams.SampleCount = Textures.History.RTGI_History.SampleCount;
		SpatialParams.Input = Radiance1;
		SpatialParams.Output = Radiance2;
		SpatialParams.Constants.Value = {
			.Size = Size,
			.StepSize = StepSize,
			.MaxSamples = MaxSamples,
			.DominationNumber = DominationSampleNumber,
		};

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RTGISpatialShader>(SpatialParams);

		Graph.AddPass("Spatial", RenderGraphPassType::Compute, Parameters, Dependencies, [SpatialParams, Size](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIDenoise, Context);

			ComputePipeline* Pipeline = GetComputePipeline<RTGISpatialShader>(Context, RTGISpatialShader::Permutation {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<RTGISpatialShader>(Pipeline, SpatialParams);
			Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { Size, 1 });
		});
	}

	static RenderGraphTextureRef SimpleDenoise(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DownsampledTextures DownTextures, RenderGraphTextureRef RTGI_Tex)
	{
		const iVector2 Size = Graph.GetTextureSize2D(RTGI_Tex);

		TextureDesc2 Desc;
		Desc.Width = (u32)Size.X;
		Desc.Height = (u32)Size.Y;
		Desc.Usage = TextureUsage::Sampled | TextureUsage::Storage;
		Desc.Format = TextureFormat::RGBA16F;

		RenderGraphTextureRef Radiance1 = RTGI_Tex;
		RenderGraphTextureRef Radiance2 = Graph.CreateTexture(Desc, "RTGI Denoiser Radiance 2");

		Desc.Format = TextureFormat::R16F;
		Graph.CreateHistoryTexture(&Textures.History.RTGI_History.SampleCount, Desc, "RTGI Denoiser Sample Count");

		static const int MaxSamples = 100;

		// temporal filter
		{
			Matrix InvViewProjection = View.CameraCur.GetViewProjection().GetInverted();
			Matrix ProjectionInv = View.CameraCur.GetProjectionMatrix().GetInverted();
			Matrix PrevViewProjection = View.CameraPrev.GetViewProjection();

			RTGITemporalShader::Parameters TemporalParams;
			TemporalParams.Velocity = DownTextures.Velocity;
			TemporalParams.Input = Radiance1;
			TemporalParams.History = Textures.History.RTGI_History.Radiance;
			TemporalParams.Depth = DownTextures.Depth;
			TemporalParams.DepthHistory = DownTextures.DepthHistory;
			TemporalParams.SampleCount = Textures.History.RTGI_History.SampleCount;
			TemporalParams.Output = Radiance2;
			TemporalParams.Constants.Value = {
				.ProjectionInv = ProjectionInv,
				.ViewProjectionInv = InvViewProjection,
				.PrevViewProjection = PrevViewProjection,
				.Size = Size,
				.MaxSamples = MaxSamples,
			};

			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<RTGITemporalShader>(TemporalParams);

			Graph.AddPass("Temporal", RenderGraphPassType::Compute, Parameters, Dependencies, [TemporalParams, Size](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIDenoise, Context);

				ComputePipeline* Pipeline = GetComputePipeline<RTGITemporalShader>(Context, RTGITemporalShader::Permutation {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<RTGITemporalShader>(Pipeline, TemporalParams);
				Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { Size, 1 });
			});
		}

		// spatial filter
		{
			SimpleDenoiseSpatial<1>(Graph, Textures, DownTextures, Size, 1, Radiance2, Radiance1, MaxSamples, 80);
			SimpleDenoiseSpatial<2>(Graph, Textures, DownTextures, Size, 2, Radiance1, Radiance2, MaxSamples, 60);
			SimpleDenoiseSpatial<3>(Graph, Textures, DownTextures, Size, 3, Radiance2, Radiance1, MaxSamples, 40);
			SimpleDenoiseSpatial<4>(Graph, Textures, DownTextures, Size, 4, Radiance1, Radiance2, MaxSamples, 20);
			SimpleDenoiseSpatial<5>(Graph, Textures, DownTextures, Size, 5, Radiance2, Radiance1, MaxSamples, 10);
			SimpleDenoiseSpatial<6>(Graph, Textures, DownTextures, Size, 6, Radiance1, Radiance2, MaxSamples, 05);
		}

		return Radiance2;
	}

	static RenderGraphTextureRef UpsampleRTGI(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DownsampledTextures DownTextures, RenderGraphTextureRef RTGI_Tex, int DownsampleFactor)
	{
		const iVector2 Size = View.RenderSize;
		const iVector2 InputSize = Graph.GetTextureSize2D(RTGI_Tex);

		TextureDesc2 Desc;
		Desc.Width = (u32)Size.X;
		Desc.Height = (u32)Size.Y;
		Desc.Usage = TextureUsage::Sampled | TextureUsage::Storage;
		Desc.Format = TextureFormat::RGBA16F;

		RenderGraphTextureRef Result = Graph.CreateTexture(Desc, "RTGI_Upsampled");

		RTGIUpsampleShader::Parameters UpsampleParams;
		UpsampleParams.Input = RTGI_Tex;
		UpsampleParams.Depth = Textures.GBufferDS;
		UpsampleParams.Normals = Textures.GBufferNormal;
		UpsampleParams.LowResDepth = DownTextures.Depth;
		UpsampleParams.Output = Result;
		UpsampleParams.Constants.Value = {
			.Size = Size,
			.InputSize = InputSize,
			.DownsampleFactor = DownsampleFactor,
		};

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RTGIUpsampleShader>(UpsampleParams);

		Graph.AddPass("Upsample", RenderGraphPassType::Compute, Parameters, Dependencies, [UpsampleParams, Size](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIDenoise, Context);

			ComputePipeline* Pipeline = GetComputePipeline<RTGIUpsampleShader>(Context, RTGIUpsampleShader::Permutation {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<RTGIUpsampleShader>(Pipeline, UpsampleParams);
			Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { Size, 1 });
		});

		return Result;
	}

	// diffuse GI, one sample
	void RayTracedGlobalIlluminationPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext, const RayTracingIrradianceVolumes::Prepared& IrradianceVolumes)
	{
		const bool UseDenoiser = View.DeferredSettings.RTGIUseDenoiser;
		const bool UseRadianceCache = View.DeferredSettings.RTGIUseRadianceCache;
		const float DiffuseBoost = View.DeferredSettings.RTGIDiffuseBoost;
		const int DownsampleFactor = View.DeferredSettings.RTGIDownsampleFactor;

		DownsampledTextures DownsampledGBuffer = DownsampleGBuffer(Graph, Textures, DownsampleFactor);

		const iVector2 GIResolution = Graph.GetTextureSize2D(DownsampledGBuffer.Depth);

		TextureDesc2 Desc{
			.Usage = TextureUsage::Storage | TextureUsage::Sampled,
			.Width = (u32)GIResolution.X,
			.Height = (u32)GIResolution.Y,
			.Format = TextureFormat::RGBA16F,
		};

		RenderGraphTextureRef RTGI_Tex = Graph.CreateTexture(Desc, "RayTracedGI");
		Graph.CreateHistoryTexture(&Textures.History.RTGI_History.Radiance, Desc, "RayTracedGI History");

		RenderPassParameters Parameters;

		RayTracedGIShader::Parameters TraceParams;
		TraceParams.Scene.UseCombinedSampler = false;
		TraceParams.Output = RTGI_Tex;
		TraceParams.GBufferNormals = DownsampledGBuffer.Normal;
		TraceParams.GBufferDepth = DownsampledGBuffer.Depth;
		TraceParams.GBufferSourcePixel = DownsampledGBuffer.SourcePixel;
		TraceParams.IrradianceVolumeConstants = IrradianceVolumes.Constants;
		TraceParams.IrradianceProbeBuffers = IrradianceVolumes.ProbeBuffers;
		TraceParams.Constants.Value = {
			.CameraPosition = View.CameraCur.Pos,
			.Random = 0,
			.DiffuseBoost = DiffuseBoost,
			.UseRadianceCache = (u32)UseRadianceCache,
			.UpscaleFactor = (float)DownsampleFactor,
		};

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RayTracedGIShader>(TraceParams);

		Graph.AddPass("RayTraceGI", RenderGraphPassType::Compute, Parameters, Dependencies, [TraceParams, GIResolution](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGI, Context);

			RayTracedGIShader::Parameters Parameters = TraceParams;
			Parameters.AccelerationStructure = Context.Scene->TLAS;
			Parameters.Constants.Value.Random = (u32)rand() % 2000;

			RayTracingPipeline* Pipeline = GetRayTracingPipeline<RayTracedGIShader>(Context, RayTracedGIShader::Permutation {}, RayTracedGIShader::PipelinePermutation {});
			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.BindRayTracingParameters<RayTracedGIShader>(Pipeline, Parameters);
			Context.CommandBuffer->TraceRays(Pipeline, GIResolution.X, GIResolution.Y, 1);
		});

		if (UseDenoiser)
		{
			RTGI_Tex = SimpleDenoise(Graph, View, Textures, DownsampledGBuffer, RTGI_Tex);
		}

		Graph.ExtractTexture(RTGI_Tex, &Textures.History.RTGI_History.Radiance);

		if (DownsampleFactor != 1)
		{
			RTGI_Tex = UpsampleRTGI(Graph, View, Textures, DownsampledGBuffer, RTGI_Tex, DownsampleFactor);
		}

		Textures.RTGI = RTGI_Tex;
	}
}
