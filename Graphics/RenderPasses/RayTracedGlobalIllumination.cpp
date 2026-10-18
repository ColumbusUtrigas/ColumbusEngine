#include "RenderPasses.h"
#include "Core/CVar.h"
#include "Graphics/ShaderCache.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace Columbus
{
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedGI);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedGIPrepare);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedGIDenoise);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedGIUpsample);

	IMPLEMENT_GPU_PROFILING_COUNTER("RTGI", "RenderGraphGPU", GpuCounterRayTracedGI);
	IMPLEMENT_GPU_PROFILING_COUNTER("RTGI Prepare", "RenderGraphGPU", GpuCounterRayTracedGIPrepare);
	IMPLEMENT_GPU_PROFILING_COUNTER("RTGI Denoise", "RenderGraphGPU", GpuCounterRayTracedGIDenoise);
	IMPLEMENT_GPU_PROFILING_COUNTER("RTGI Upsample", "RenderGraphGPU", GpuCounterRayTracedGIUpsample);

	ConsoleVariable<float> CVar_RTGIMaxRayIntensity("r.RTGI.MaxRayIntensity", "Maximum incoming GI ray radiance component before RGB SH projection; 0 disables the biased firefly limit", 64.0f);

	DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounterRTGIHistory);
	IMPLEMENT_MEMORY_PROFILING_COUNTER("RTGI histories", "RTGI", MemoryCounterRTGIHistory);

	static void DestroyHistoryTexture(SPtr<DeviceVulkan> Device, Texture2*& Texture)
	{
		if (Texture != nullptr)
		{
			RemoveProfilingMemory(MemoryCounterRTGIHistory, Texture->GetSize());
			Device->DestroyTexture(Texture);
			Texture = nullptr;
		}
	}

	void RTGIDenoiserHistory::Destroy(SPtr<DeviceVulkan> Device)
	{
		for (RTGIFrameHistory& Frame : Frames)
		{
			DestroyHistoryTexture(Device, Frame.ProbeRadianceSH);
			DestroyHistoryTexture(Device, Frame.SampleCount);
			DestroyHistoryTexture(Device, Frame.Depth);
			DestroyHistoryTexture(Device, Frame.Normals);
			DestroyHistoryTexture(Device, Frame.SurfaceData);
			DestroyHistoryTexture(Device, Frame.UpsampledRadiance);
			DestroyHistoryTexture(Device, Frame.UpsampledNormals);
		}

		Valid = false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Shader interfaces

	struct RTGIGeometryConstants
	{
		Matrix InverseViewProjection;
		Matrix PreviousInverseViewProjection;
		Vector4 CameraPositionPixelSize;
		Vector4 JitterDelta;
		Vector4 DepthProjection;
		iVector4 Sizes;
		iVector4 Config;
	};

	static_assert(sizeof(RTGIGeometryConstants) == 208);

	struct RTGI_Parameters
	{
		Vector3 CameraPosition{};
		u32     Random = 0;
		float   DiffuseBoost = 1;
		u32     UseRadianceCache = 0;
		float   MaxRayIntensity = 64;
	};


	struct ProbeGeometry
	{
		RenderGraphTextureRef Depth;
		RenderGraphTextureRef Normal;
		RenderGraphTextureRef SurfaceData;
	};

	struct ProbeRays
	{
		RenderGraphTextureRef Radiance;
		RenderGraphTextureRef Direction;
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
			ShaderStorageTexture OutputDepth;
			ShaderStorageTexture OutputNormals;
			ShaderStorageTexture OutputSurfaceData;
			ShaderConstants<RTGIGeometryConstants> Geometry;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Depth, 0, 0);
			Binder.Bind(Params.Normals, 0, 1);
			Binder.Bind(Params.OutputDepth, 0, 2);
			Binder.Bind(Params.OutputNormals, 0, 3);
			Binder.Bind(Params.OutputSurfaceData, 0, 4);
			Binder.Bind(Params.Geometry, 0, 20);
		}
	};

	struct RTGITemporalShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedGlobalIllumination/Temporal.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderSampledTexture Input;
			ShaderSampledTexture InputDirection;
			ShaderSampledTexture HistorySH;
			ShaderSampledTexture Depth;
			ShaderSampledTexture Normals;
			ShaderSampledTexture SurfaceData;
			ShaderSampledTexture PreviousDepth;
			ShaderSampledTexture PreviousNormals;
			ShaderSampledTexture PreviousSurfaceData;
			ShaderSampledTexture SampleCountHistory;
			ShaderSampledTexture Velocity;
			ShaderStorageTexture OutputSH;
			ShaderStorageTexture SampleCount;
			ShaderConstants<RTGIGeometryConstants> Geometry;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Input, 0, 0);
			Binder.Bind(Params.InputDirection, 0, 1);
			Binder.Bind(Params.HistorySH, 0, 2);
			Binder.Bind(Params.Depth, 0, 3);
			Binder.Bind(Params.Normals, 0, 4);
			Binder.Bind(Params.SurfaceData, 0, 5);
			Binder.Bind(Params.PreviousDepth, 0, 6);
			Binder.Bind(Params.PreviousNormals, 0, 7);
			Binder.Bind(Params.PreviousSurfaceData, 0, 8);
			Binder.Bind(Params.SampleCountHistory, 0, 9);
			Binder.Bind(Params.Velocity, 0, 10);
			Binder.Bind(Params.OutputSH, 0, 11);
			Binder.Bind(Params.SampleCount, 0, 12);
			Binder.Bind(Params.Geometry, 0, 20);
		}
	};

	struct RTGISpatialShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedGlobalIllumination/Spatial.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderSampledTexture InputSH;
			ShaderSampledTexture Depth;
			ShaderSampledTexture Normals;
			ShaderSampledTexture SurfaceData;
			ShaderStorageTexture OutputSH;
			ShaderSampledTexture SampleCount;
			ShaderConstants<RTGIGeometryConstants> Geometry;
			ShaderPushConstants<int> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.InputSH, 0, 0);
			Binder.Bind(Params.Depth, 0, 1);
			Binder.Bind(Params.Normals, 0, 2);
			Binder.Bind(Params.SurfaceData, 0, 3);
			Binder.Bind(Params.OutputSH, 0, 4);
			Binder.Bind(Params.SampleCount, 0, 5);
			Binder.Bind(Params.Geometry, 0, 20);
			Binder.Bind(Params.Constants);
		}
	};

	struct RTGIUpsampleConstants
	{
		u32 HistoryValid = 0;
		u32 UseDenoiser  = 1;
	};

	static_assert(sizeof(RTGIUpsampleConstants) == 8);

	struct RTGIUpsampleShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedGlobalIllumination/Upsample.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderSampledTexture InputSH;
			ShaderSampledTexture Depth { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture Normals;
			ShaderSampledTexture SampleDepth;
			ShaderSampledTexture SampleNormals;
			ShaderSampledTexture SurfaceData;
			ShaderStorageTexture Output;
			ShaderSampledTexture PreviousRadiance;
			ShaderSampledTexture PreviousDepth { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture PreviousNormals;
			ShaderSampledTexture Velocity;
			ShaderSampledTexture ShadingNormals;
			ShaderSampledTexture RayInput;
			ShaderSampledTexture RayDirection;
			ShaderSampledTexture SampleCount;
			ShaderConstants<RTGIGeometryConstants> Geometry;
			ShaderPushConstants<RTGIUpsampleConstants> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.InputSH, 0, 0);
			Binder.Bind(Params.Depth, 0, 1);
			Binder.Bind(Params.Normals, 0, 2);
			Binder.Bind(Params.SampleDepth, 0, 3);
			Binder.Bind(Params.SampleNormals, 0, 4);
			Binder.Bind(Params.SurfaceData, 0, 5);
			Binder.Bind(Params.Output, 0, 6);
			Binder.Bind(Params.PreviousRadiance, 0, 7);
			Binder.Bind(Params.PreviousDepth, 0, 8);
			Binder.Bind(Params.PreviousNormals, 0, 9);
			Binder.Bind(Params.Velocity, 0, 10);
			Binder.Bind(Params.ShadingNormals, 0, 11);
			Binder.Bind(Params.RayInput, 0, 12);
			Binder.Bind(Params.RayDirection, 0, 13);
			Binder.Bind(Params.SampleCount, 0, 14);
			Binder.Bind(Params.Geometry, 0, 20);
			Binder.Bind(Params.Constants);
		}
	};

	struct RayTracedGIShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedGlobalIllumination/RayTracedGI.csd";

		struct Permutation
		{
			int UseSharc = 0;
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
			Builder.Axis("USE_SHARC", &Permutation::UseSharc);
		}

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
			RadianceCache::QueryParameters Cache;
			ShaderAccelerationStructure AccelerationStructure;
			ShaderStorageTexture Output;
			ShaderStorageTexture OutputDirection;
			ShaderSampledTexture SampleNormals;
			ShaderSampledTexture GBufferDepth;
			ShaderSampledTexture GBufferSurfaceData;
			ShaderConstants<IrradianceVolumes::Constants> IrradianceVolumeConstants;
			std::array<ShaderReadBuffer, IrradianceVolumes::MaxVolumes> IrradianceProbeBuffers;
			ShaderPushConstants<RTGI_Parameters> Constants { {}, ShaderType::Raygen };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene);
			Params.Cache.Bind(Binder);
			Binder.Bind(Params.AccelerationStructure, 2, 0);
			Binder.Bind(Params.Output, 2, 1);
			Binder.Bind(Params.OutputDirection, 2, 5);
			Binder.Bind(Params.SampleNormals, 2, 2);
			Binder.Bind(Params.GBufferDepth, 2, 3);
			Binder.Bind(Params.GBufferSurfaceData, 2, 4);
			Binder.Bind(Params.IrradianceVolumeConstants, 2, 9);
			ShaderArray<ShaderReadBuffer> IrradianceProbeBuffers;
			IrradianceProbeBuffers.Set(Params.IrradianceProbeBuffers.data(), IrradianceVolumes::MaxVolumes);
			Binder.Bind(IrradianceProbeBuffers, 2, 10);
			Binder.Bind(Params.Constants);
		}
	};


	///////////////////////////////////////////////////////////////////////////////////////////////
	// Persistent probe resources and surface placement

	static TextureDesc2 MakeRTGITextureDesc(iVector2 Size, TextureFormat Format)
	{
		return TextureDesc2 {
			.Usage  = TextureUsage::StorageSampled,
			.Width  = (u32)Size.X,
			.Height = (u32)Size.Y,
			.Format = Format,
		};
	}

	static TextureDesc2 MakeRTGIGeometryDesc(iVector2 Size)
	{
		TextureDesc2 Desc = MakeRTGITextureDesc(Size, TextureFormat::RG16F);
		Desc.Usage = Desc.Usage | TextureUsage::RenderTargetColor;
		return Desc;
	}

	static RenderGraphTextureRef ImportHistoryTexture(RenderGraph& Graph, Texture2* Texture, const char* Name)
	{
		return Graph.RegisterExternalTexture(SPtr<Texture2>(Texture, [](Texture2*) {}), Name);
	}

	static bool PrepareHistoryTexture(RenderGraph& Graph, Texture2*& Texture, const TextureDesc2& Desc, const char* Name)
	{
		const u64 PreviousSize = Texture != nullptr ? Texture->GetSize() : 0;
		const bool Invalidated = Graph.CreateHistoryTexture(&Texture, Desc, Name);

		if (Invalidated)
		{
			RemoveProfilingMemory(MemoryCounterRTGIHistory, PreviousSize);
			AddProfilingMemory(MemoryCounterRTGIHistory, Texture->GetSize());
		}

		return Invalidated;
	}

	RenderGraphTextureRef RTGIDenoiserHistory::PrepareGeometryTarget(RenderGraph& Graph, iVector2 Size, bool UseDenoiser)
	{
		const TextureDesc2 Desc = MakeRTGIGeometryDesc(Size);

		if (!UseDenoiser)
		{
			return Graph.CreateTexture(Desc, "RTGI Geometric Normals");
		}

		for (RTGIFrameHistory& Frame : Frames)
		{
			if (PrepareHistoryTexture(Graph, Frame.UpsampledNormals, Desc, "RTGI Receiver Normal History"))
			{
				Valid = false;
			}
		}

		return ImportHistoryTexture(Graph, Frames[FrameIndex & 1].UpsampledNormals, "RTGI Geometric Normals");
	}


	static TextureDesc2 MakeProbeSHDesc(iVector2 ProbeSize)
	{
		// Seven RGBA rows hold all 27 RGB SH coefficients and mean hit distance.
		return MakeRTGITextureDesc({ ProbeSize.X, ProbeSize.Y * 7 }, TextureFormat::RGBA16F);
	}

	static bool PrepareHistories(RenderGraph& Graph, RTGIDenoiserHistory& History, iVector2 Size, iVector2 ProbeSize)
	{
		bool Invalidated = false;

		for (RTGIFrameHistory& Frame : History.Frames)
		{
			Invalidated |= PrepareHistoryTexture(Graph, Frame.ProbeRadianceSH, MakeProbeSHDesc(ProbeSize), "RTGI Probe SH History");
			Invalidated |= PrepareHistoryTexture(Graph, Frame.SampleCount, MakeRTGITextureDesc(ProbeSize, TextureFormat::R16F), "RTGI Sample Count History");
			Invalidated |= PrepareHistoryTexture(Graph, Frame.Depth, MakeRTGITextureDesc(ProbeSize, TextureFormat::R32F), "RTGI Depth History");
			Invalidated |= PrepareHistoryTexture(Graph, Frame.Normals, MakeRTGITextureDesc(ProbeSize, TextureFormat::RGBA16F), "RTGI Normal History");
			Invalidated |= PrepareHistoryTexture(Graph, Frame.SurfaceData, MakeRTGITextureDesc(ProbeSize, TextureFormat::RGBA32F), "RTGI Surface History");
			Invalidated |= PrepareHistoryTexture(Graph, Frame.UpsampledRadiance, MakeRTGITextureDesc(Size, TextureFormat::RGBA16F), "RTGI Receiver History");
			Invalidated |= PrepareHistoryTexture(Graph, Frame.UpsampledNormals, MakeRTGIGeometryDesc(Size), "RTGI Receiver Normal History");
		}

		return Invalidated;
	}

	static ProbeGeometry PrepareProbes(RenderGraph& Graph, SceneTextures& Textures, const RTGIGeometryConstants& Geometry, RTGIFrameHistory* History)
	{
		const iVector2 Size(Geometry.Sizes.Z, Geometry.Sizes.W);
		ProbeGeometry Result {
			.Depth       = History != nullptr ? ImportHistoryTexture(Graph, History->Depth, "RTGI Sample Depth") : Graph.CreateTexture(MakeRTGITextureDesc(Size, TextureFormat::R32F), "RTGI Sample Depth"),
			.Normal      = History != nullptr ? ImportHistoryTexture(Graph, History->Normals, "RTGI Sample Normals") : Graph.CreateTexture(MakeRTGITextureDesc(Size, TextureFormat::RGBA16F), "RTGI Sample Normals"),
			.SurfaceData = History != nullptr ? ImportHistoryTexture(Graph, History->SurfaceData, "RTGI Sample Surface Data") : Graph.CreateTexture(MakeRTGITextureDesc(Size, TextureFormat::RGBA32F), "RTGI Sample Surface Data"),
		};

		RTGIDownsampleShader::Parameters Downsample;
		Downsample.Depth             = Textures.GBufferDS;
		Downsample.Normals           = Textures.GBufferGeometricNormal;
		Downsample.OutputDepth       = Result.Depth;
		Downsample.OutputNormals     = Result.Normal;
		Downsample.OutputSurfaceData = Result.SurfaceData;
		Downsample.Geometry.Value    = Geometry;

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RTGIDownsampleShader>(Downsample);

		Graph.AddPass("RTGI Prepare Probes", RenderGraphPassType::Compute, Parameters, Dependencies, [Downsample, Size](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIPrepare, Context);
			ComputePipeline* Pipeline = GetComputePipeline<RTGIDownsampleShader>(Context, {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<RTGIDownsampleShader>(Pipeline, Downsample);
			Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { Size, 1 });
		});

		return Result;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Directional probe accumulation and filtering

	static RenderGraphTextureRef FilterProbes(RenderGraph& Graph, SceneTextures& Textures, const ProbeGeometry& Guides, ProbeRays Rays, RenderGraphTextureRef Count, const RTGIGeometryConstants& Geometry, RTGIFrameHistory& Current, const RTGIFrameHistory& Previous)
	{
		const iVector2 Size(Geometry.Sizes.Z, Geometry.Sizes.W);
		RenderGraphTextureRef Result = Graph.CreateTexture(MakeProbeSHDesc(Size), "RTGI Temporal SH");
		RTGITemporalShader::Parameters Temporal;
		Temporal.Input               = Rays.Radiance;
		Temporal.InputDirection      = Rays.Direction;
		Temporal.HistorySH           = ImportHistoryTexture(Graph, Previous.ProbeRadianceSH, "RTGI Previous Probe SH");
		Temporal.Depth               = Guides.Depth;
		Temporal.Normals             = Guides.Normal;
		Temporal.SurfaceData         = Guides.SurfaceData;
		Temporal.PreviousDepth       = ImportHistoryTexture(Graph, Previous.Depth, "RTGI Previous Depth");
		Temporal.PreviousNormals     = ImportHistoryTexture(Graph, Previous.Normals, "RTGI Previous Normals");
		Temporal.PreviousSurfaceData = ImportHistoryTexture(Graph, Previous.SurfaceData, "RTGI Previous Surface Data");
		Temporal.SampleCountHistory  = ImportHistoryTexture(Graph, Previous.SampleCount, "RTGI Previous Sample Count");
		Temporal.Velocity            = Textures.Velocity;
		Temporal.OutputSH            = Result;
		Temporal.SampleCount         = Count;
		Temporal.Geometry.Value      = Geometry;

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RTGITemporalShader>(Temporal);

		Graph.AddPass("RTGI Temporal Probes", RenderGraphPassType::Compute, Parameters, Dependencies, [Temporal, Size](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIDenoise, Context);
			ComputePipeline* Pipeline = GetComputePipeline<RTGITemporalShader>(Context, {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<RTGITemporalShader>(Pipeline, Temporal);
			Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { Size, 1 });
		});

		static const char* Names[3] = { "RTGI Probe SH", "RTGI Probe SH 2", "RTGI Probe SH 4" };

		for (int i = 0; i < 3; ++i)
		{
			// Only the narrow first pass feeds probe history. Broad disocclusion
			// repair stays display-only, and sample counts retain their true age.
			RenderGraphTextureRef Filtered = i == 0 ? ImportHistoryTexture(Graph, Current.ProbeRadianceSH, Names[i]) : Graph.CreateTexture(MakeProbeSHDesc(Size), Names[i]);
			RTGISpatialShader::Parameters Spatial;
			Spatial.InputSH         = Result;
			Spatial.Depth           = Guides.Depth;
			Spatial.Normals         = Guides.Normal;
			Spatial.SurfaceData     = Guides.SurfaceData;
			Spatial.OutputSH        = Filtered;
			Spatial.SampleCount     = Count;
			Spatial.Geometry.Value  = Geometry;
			Spatial.Constants.Value = 1 << i;

			RenderPassDependencies SpatialDependencies(Graph.Allocator);
			SpatialDependencies.Bind<RTGISpatialShader>(Spatial);

			Graph.AddPass("RTGI Filter Probe SH", RenderGraphPassType::Compute, Parameters, SpatialDependencies, [Spatial, Size](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIDenoise, Context);
				ComputePipeline* Pipeline = GetComputePipeline<RTGISpatialShader>(Context, {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.BindComputeParameters<RTGISpatialShader>(Pipeline, Spatial);
				Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { Size, 1 });
			});

			Result = Filtered;
		}

		return Result;
	}

	static RenderGraphTextureRef IntegrateProbes(RenderGraph& Graph, SceneTextures& Textures, const ProbeGeometry& Guides, ProbeRays Rays, RenderGraphTextureRef SH, RenderGraphTextureRef Count, const RTGIGeometryConstants& Geometry, RTGIFrameHistory* Current, const RTGIFrameHistory& Previous, const RTGIUpsampleConstants& Constants)
	{
		const iVector2 Size(Geometry.Sizes.X, Geometry.Sizes.Y);
		RenderGraphTextureRef Result = Current != nullptr ? ImportHistoryTexture(Graph, Current->UpsampledRadiance, "RTGI_Upsampled") : Graph.CreateTexture(MakeRTGITextureDesc(Size, TextureFormat::RGBA16F), "RTGI_Upsampled");
		RTGIUpsampleShader::Parameters Integrate;
		Integrate.InputSH          = SH;
		Integrate.Depth            = Textures.GBufferDS;
		Integrate.Normals          = Textures.GBufferGeometricNormal;
		Integrate.SampleDepth      = Guides.Depth;
		Integrate.SampleNormals    = Guides.Normal;
		Integrate.SurfaceData      = Guides.SurfaceData;
		Integrate.Output           = Result;
		Integrate.PreviousRadiance = Rays.Radiance;
		Integrate.PreviousDepth    = Textures.GBufferDS;
		Integrate.PreviousNormals  = Textures.GBufferGeometricNormal;
		Integrate.Velocity         = Textures.Velocity;
		Integrate.ShadingNormals   = Textures.GBufferNormal;
		Integrate.RayInput         = Rays.Radiance;
		Integrate.RayDirection     = Rays.Direction;
		Integrate.SampleCount      = Count;
		Integrate.Geometry.Value   = Geometry;
		Integrate.Constants.Value  = Constants;

		if (Current != nullptr)
		{
			Integrate.PreviousRadiance = ImportHistoryTexture(Graph, Previous.UpsampledRadiance, "RTGI Previous Receiver Lighting");
			Integrate.PreviousDepth    = ImportHistoryTexture(Graph, Textures.History.Depth, "RTGI Previous Receiver Depth");
			Integrate.PreviousNormals  = ImportHistoryTexture(Graph, Previous.UpsampledNormals, "RTGI Previous Receiver Normals");
		}

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RTGIUpsampleShader>(Integrate);

		Graph.AddPass("RTGI Integrate Probes", RenderGraphPassType::Compute, Parameters, Dependencies, [Integrate, Size](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIUpsample, Context);
			ComputePipeline* Pipeline = GetComputePipeline<RTGIUpsampleShader>(Context, {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<RTGIUpsampleShader>(Pipeline, Integrate);
			Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { Size, 1 });
		});

		return Result;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// GI tracing and frame history

	void RayTracedGlobalIlluminationPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext)
	{
		RTGIDenoiserHistory& History = Textures.History.RTGI_History;
		const bool UseDenoiser = View.DeferredSettings.RTGIUseDenoiser;
		const bool UseRadianceCache = View.DeferredSettings.RTGIUseRadianceCache && Textures.RadianceCache.Query.Enabled;
		const float DiffuseBoost = View.DeferredSettings.RTGIDiffuseBoost;
		const int DownsampleFactor = std::clamp(View.DeferredSettings.RTGIDownsampleFactor, 1, 8);
		const iVector2 Size = View.RenderSize;
		const iVector2 TileSize = (Size + DownsampleFactor - 1) / DownsampleFactor;

		const Matrix ViewProjection = View.CameraCur.GetViewProjection();
		const Matrix ExpectedPrevious = View.CameraPrev.GetViewProjection();
		const bool CameraHistoryMatches = std::memcmp(&History.ViewProjection, &ExpectedPrevious, sizeof(Matrix)) == 0;
		const bool SamplingMatches = History.DownsampleFactor == DownsampleFactor;
		const bool LightingMatches = History.UseRadianceCache == UseRadianceCache && History.DiffuseBoost == DiffuseBoost;
		const bool Invalidated = UseDenoiser && PrepareHistories(Graph, History, Size, TileSize);
		const bool ValidHistory = UseDenoiser && History.Valid && CameraHistoryMatches && SamplingMatches && LightingMatches && !Invalidated;
		RTGIFrameHistory& Current = History.Frames[History.FrameIndex & 1];
		const RTGIFrameHistory& Previous = History.Frames[(History.FrameIndex & 1) ^ 1];

		const Matrix& Projection = View.CameraCur.GetProjectionMatrix();
		const Matrix& PreviousProjection = View.CameraPrev.GetProjectionMatrix();
		const Matrix ProjectionInv = Projection.GetInverted();
		const Vector2 JitterDelta = View.CameraPrev.GetJittering() - View.CameraCur.GetJittering();
		RTGIGeometryConstants Geometry;
		Geometry.InverseViewProjection         = ViewProjection.GetInverted();
		Geometry.PreviousInverseViewProjection = History.InverseViewProjection;
		Geometry.CameraPositionPixelSize       = Vector4(View.CameraCur.Pos, 2 * std::abs(ProjectionInv.M[1][1]) / float(Size.Y));
		Geometry.JitterDelta                   = Vector4(JitterDelta.X * 0.5f * Size.X, -JitterDelta.Y * 0.5f * Size.Y, 0, 0);
		Geometry.DepthProjection               = Vector4(Projection.M[2][2], Projection.M[2][3], PreviousProjection.M[2][2], PreviousProjection.M[2][3]);
		Geometry.Sizes                         = iVector4(Size.X, Size.Y, TileSize.X, TileSize.Y);
		Geometry.Config                        = iVector4(DownsampleFactor, 1, ValidHistory, History.FrameIndex++);

		const ProbeGeometry Guides = PrepareProbes(Graph, Textures, Geometry, UseDenoiser ? &Current : nullptr);
		ProbeRays Rays {
			Graph.CreateTexture(MakeRTGITextureDesc(TileSize, TextureFormat::RGBA16F), "RayTracedGI"),
			Graph.CreateTexture(MakeRTGITextureDesc(TileSize, TextureFormat::RGBA16F), "RTGI Direction"),
		};

		RayTracedGIShader::Parameters Trace;
		Trace.Scene.UseCombinedSampler = false;
		Trace.Cache = Textures.RadianceCache.Query;
		Trace.Cache.Enabled = UseRadianceCache;

		Trace.Output          = Rays.Radiance;
		Trace.OutputDirection = Rays.Direction;

		Trace.SampleNormals      = Guides.Normal;
		Trace.GBufferDepth       = Guides.Depth;
		Trace.GBufferSurfaceData = Guides.SurfaceData;

		Trace.IrradianceVolumeConstants = DeferredContext.BakedLighting.IrradianceVolumes.Constants;
		Trace.IrradianceProbeBuffers = DeferredContext.BakedLighting.IrradianceVolumes.ProbeBuffers;
		Trace.Constants.Value = {
			.CameraPosition   = View.CameraCur.Pos,
			.Random           = (u32)Geometry.Config.W,
			.DiffuseBoost     = DiffuseBoost,
			.UseRadianceCache = (u32)UseRadianceCache,
			.MaxRayIntensity  = std::max(0.0f, CVar_RTGIMaxRayIntensity.GetValue()),
		};

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RayTracedGIShader>(Trace);

		Graph.AddPass("RayTraceGI", RenderGraphPassType::Compute, Parameters, Dependencies, [Trace, TileSize](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGI, Context);
			RayTracedGIShader::Parameters Bound = Trace;
			Bound.AccelerationStructure = Context.Scene->TLAS;
			RayTracingPipeline* Pipeline = GetRayTracingPipeline<RayTracedGIShader>(Context, { Trace.Cache.Enabled ? 1 : 0 }, RayTracedGIShader::PipelinePermutation {});
			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.BindRayTracingParameters<RayTracedGIShader>(Pipeline, Bound);
			Context.CommandBuffer->TraceRays(Pipeline, TileSize.X, TileSize.Y, 1);
		});

		RenderGraphTextureRef ProbeSH = Rays.Radiance;
		RenderGraphTextureRef Count   = Guides.Depth; // Valid dummy without probe filtering.

		if (UseDenoiser)
		{
			Count = ImportHistoryTexture(Graph, Current.SampleCount, "RTGI Sample Count");
			ProbeSH = FilterProbes(Graph, Textures, Guides, Rays, Count, Geometry, Current, Previous);
		}

		const RTGIUpsampleConstants IntegrateConstants { (u32)ValidHistory, (u32)UseDenoiser };
		Textures.RTGI = IntegrateProbes(Graph, Textures, Guides, Rays, ProbeSH, Count, Geometry, UseDenoiser ? &Current : nullptr, Previous, IntegrateConstants);
		History.InverseViewProjection = Geometry.InverseViewProjection;
		History.ViewProjection        = ViewProjection;
		History.DownsampleFactor      = DownsampleFactor;
		History.UseRadianceCache      = UseRadianceCache;
		History.DiffuseBoost          = DiffuseBoost;
		History.Valid                 = UseDenoiser;
	}
}
