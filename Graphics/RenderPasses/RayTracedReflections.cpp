#include "RenderPasses.h"
#include "RayTracingIrradianceVolumes.h"
#include "Core/Core.h"
#include "Core/CVar.h"
#include "Graphics/ShaderCache.h"

namespace Columbus
{

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedReflections);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedReflectionsDenoise);

	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Reflections", "RenderGraphGPU", GpuCounterRayTracedReflections);
	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Reflections Denoise", "RenderGraphGPU", GpuCounterRayTracedReflectionsDenoise);

	ConsoleVariable<float> CVar_MaxRoughness("r.RTReflection.MaxRoughness", "Max roughness to trace, default - 0.5", 0.5f);
	ConsoleVariable<int> CVar_Denoiser("r.RTReflection.Denoiser", "Reflection denoiser: 0 custom, 1 FFX-style", 1);
	ConsoleVariable<bool> CVar_RayTracingIrradianceVolumes("r.RayTracing.IrradianceVolumes", "Sample baked irradiance volumes from ray traced GI/reflections", true);

	struct RayTracedReflectionPassParameters
	{
		Vector4 CameraPosition{};
		float MaxRoughness = 0.5f;
		u32 Random = 0;
		u32 UseRadianceCache = 0;
	};

	struct RayTracedReflectionsResolveParameters
	{
		iVector2 ImageSize{};
		iVector2 _Padding{};
	};

	struct RayTracedReflectionsTemporalParameters
	{
		iVector2 Size{};
		int MaxSamples = 0;
		int _Padding = 0;
	};

	struct RTReflectionDenoiserConstants
	{
		Matrix InvViewProjection;
		Matrix Projection;
		Matrix InvProjection;
		Matrix View;
		Matrix InvView;
		Matrix PrevViewProjection;
		u32 BufferDimensions[2];
		float InvBufferDimensions[2];
		float TemporalStabilityFactor;
		float DepthBufferThickness;
		float RoughnessThreshold;
		float VarianceThreshold;
		u32 FrameIndex;
		u32 MaxTraversalIntersections;
		u32 MinTraversalOccupancy;
		u32 MostDetailedMip;
		u32 SamplesPerQuad;
		u32 TemporalVarianceGuidedTracingEnabled;
	};

	struct ReflectionDenoiserReprojectShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/DenoiserReflection/Reproject.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderReadBuffer Constants;
			ShaderSampledTexture Depth { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture Roughness;
			ShaderSampledTexture Normal;
			ShaderSampledTexture DepthHistory { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture RoughnessHistory;
			ShaderSampledTexture NormalHistory;
			ShaderSampledTexture InputRadiance;
			ShaderSampledTexture RadianceHistory;
			ShaderSampledTexture Velocity;
			ShaderSampledTexture AverageRadianceHistory;
			ShaderSampledTexture VarianceHistory;
			ShaderSampledTexture SampleCountHistory;
			ShaderStaticSampler LinearSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderStorageTexture ReprojectedRadiance;
			ShaderStorageTexture AverageRadiance;
			ShaderStorageTexture Variance;
			ShaderStorageTexture SampleCount;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Constants, 0, 0);
			Binder.Bind(Params.Depth, 1, 0);
			Binder.Bind(Params.Roughness, 1, 1);
			Binder.Bind(Params.Normal, 1, 2);
			Binder.Bind(Params.DepthHistory, 1, 3);
			Binder.Bind(Params.RoughnessHistory, 1, 4);
			Binder.Bind(Params.NormalHistory, 1, 5);
			Binder.Bind(Params.InputRadiance, 1, 6);
			Binder.Bind(Params.RadianceHistory, 1, 7);
			Binder.Bind(Params.Velocity, 1, 8);
			Binder.Bind(Params.AverageRadianceHistory, 1, 9);
			Binder.Bind(Params.VarianceHistory, 1, 10);
			Binder.Bind(Params.SampleCountHistory, 1, 11);
			Binder.Bind(Params.LinearSampler, 1, 12);
			Binder.Bind(Params.ReprojectedRadiance, 1, 13);
			Binder.Bind(Params.AverageRadiance, 1, 14);
			Binder.Bind(Params.Variance, 1, 15);
			Binder.Bind(Params.SampleCount, 1, 16);
		}
	};

	struct ReflectionDenoiserPrefilterShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/DenoiserReflection/Prefilter.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderReadBuffer Constants;
			ShaderSampledTexture Depth { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture Roughness;
			ShaderSampledTexture Normal;
			ShaderSampledTexture AverageRadiance;
			ShaderSampledTexture InputRadiance;
			ShaderSampledTexture Variance;
			ShaderSampledTexture SampleCount;
			ShaderStaticSampler LinearSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderStorageTexture OutputRadiance;
			ShaderStorageTexture OutputVariance;
			ShaderStorageTexture OutputSampleCount;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Constants, 0, 0);
			Binder.Bind(Params.Depth, 1, 0);
			Binder.Bind(Params.Roughness, 1, 1);
			Binder.Bind(Params.Normal, 1, 2);
			Binder.Bind(Params.AverageRadiance, 1, 3);
			Binder.Bind(Params.InputRadiance, 1, 4);
			Binder.Bind(Params.Variance, 1, 5);
			Binder.Bind(Params.SampleCount, 1, 6);
			Binder.Bind(Params.LinearSampler, 1, 7);
			Binder.Bind(Params.OutputRadiance, 1, 8);
			Binder.Bind(Params.OutputVariance, 1, 9);
			Binder.Bind(Params.OutputSampleCount, 1, 10);
		}
	};

	struct ReflectionDenoiserResolveTemporalShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/DenoiserReflection/ResolveTemporal.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderReadBuffer Constants;
			ShaderSampledTexture Roughness;
			ShaderSampledTexture AverageRadiance;
			ShaderSampledTexture InputRadiance;
			ShaderSampledTexture ReprojectedRadiance;
			ShaderSampledTexture Variance;
			ShaderSampledTexture SampleCount;
			ShaderStaticSampler LinearSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderStorageTexture OutputRadiance;
			ShaderStorageTexture OutputVariance;
			ShaderStorageTexture OutputSampleCount;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Constants, 0, 0);
			Binder.Bind(Params.Roughness, 1, 0);
			Binder.Bind(Params.AverageRadiance, 1, 1);
			Binder.Bind(Params.InputRadiance, 1, 2);
			Binder.Bind(Params.ReprojectedRadiance, 1, 3);
			Binder.Bind(Params.Variance, 1, 4);
			Binder.Bind(Params.SampleCount, 1, 5);
			Binder.Bind(Params.LinearSampler, 1, 6);
			Binder.Bind(Params.OutputRadiance, 1, 7);
			Binder.Bind(Params.OutputVariance, 1, 8);
			Binder.Bind(Params.OutputSampleCount, 1, 9);
		}
	};

	struct RayTracedReflectionsShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedReflections/RayTraceReflections.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct PipelinePermutation
		{
			u32 MaxRecursionDepth = 1;
		};

		static RayTracingPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			RayTracingPipelineDesc Desc {};
			Desc.Name = "RayTracedReflectionsPass";
			Desc.MaxRecursionDepth = Permutation.MaxRecursionDepth;
			return Desc;
		}

		struct Parameters
		{
			ShaderGPUScene Scene;
			ShaderAccelerationStructure AccelerationStructure;
			ShaderStorageTexture Output;
			ShaderStorageTexture ResultDirectionDistance;
			ShaderStorageTexture ResultRayPdf;
			ShaderSampledTexture GBufferAlbedo;
			ShaderSampledTexture GBufferNormals;
			ShaderSampledTexture GBufferRoughnessMetallic;
			ShaderSampledTexture GBufferDepth { TextureBindingFlags::AspectDepth };
			ShaderConstants<RayTracingIrradianceVolumes::Constants> IrradianceVolumeConstants;
			std::array<ShaderReadBuffer, RayTracingIrradianceVolumes::MaxVolumes> IrradianceProbeBuffers;
			ShaderPushConstants<RayTracedReflectionPassParameters> Constants { {}, ShaderType::Raygen };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.AccelerationStructure, 2, 0);
			Binder.Bind(Params.Output, 2, 1);
			Binder.Bind(Params.ResultDirectionDistance, 2, 2);
			Binder.Bind(Params.ResultRayPdf, 2, 3);
			Binder.Bind(Params.GBufferAlbedo, 2, 4);
			Binder.Bind(Params.GBufferNormals, 2, 5);
			Binder.Bind(Params.GBufferRoughnessMetallic, 2, 6);
			Binder.Bind(Params.GBufferDepth, 2, 7);
			Binder.Bind(Params.IrradianceVolumeConstants, 2, 9);
			ShaderArray<ShaderReadBuffer> IrradianceProbeBuffers;
			IrradianceProbeBuffers.Set(Params.IrradianceProbeBuffers.data(), RayTracingIrradianceVolumes::MaxVolumes);
			Binder.Bind(IrradianceProbeBuffers, 2, 10);
			Binder.Bind(Params.Constants);
		}
	};

	struct ReflectionsResolveShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedReflections/ReflectionsResolve.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderSampledTexture Radiance;
			ShaderSampledTexture Rays;
			ShaderSampledTexture RayPdf;
			ShaderSampledTexture GBufferAlbedo;
			ShaderSampledTexture GBufferNormal;
			ShaderSampledTexture GBufferRM;
			ShaderSampledTexture GBufferDepth { TextureBindingFlags::AspectDepth };
			ShaderStorageTexture Output;
			ShaderStorageTexture OutputHitDistance;
			ShaderReadBuffer GPUSceneScene;
			ShaderPushConstants<RayTracedReflectionsResolveParameters> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Radiance, 0, 0);
			Binder.Bind(Params.Rays, 0, 1);
			Binder.Bind(Params.RayPdf, 0, 2);
			Binder.Bind(Params.GBufferAlbedo, 0, 3);
			Binder.Bind(Params.GBufferNormal, 0, 4);
			Binder.Bind(Params.GBufferRM, 0, 5);
			Binder.Bind(Params.GBufferDepth, 0, 6);
			Binder.Bind(Params.Output, 0, 7);
			Binder.Bind(Params.OutputHitDistance, 0, 8);
			Binder.Bind(Params.GPUSceneScene, 0, 9);
			Binder.Bind(Params.Constants);
		}
	};

	struct ReflectionsTemporalShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RayTracedReflections/ReflectionsTemporal.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderSampledTexture Current;
			ShaderSampledTexture History;
			ShaderSampledTexture Velocity;
			ShaderSampledTexture Depth { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture DepthHistory { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture HistorySampleCount;
			ShaderSampledTexture Rays;
			ShaderSampledTexture ResolvedHitDistance;
			ShaderSampledTexture Normal;
			ShaderSampledTexture RoughnessMetallic;
			ShaderSampledTexture NormalHistory;
			ShaderSampledTexture RoughnessMetallicHistory;
			ShaderStorageTexture OutputRadiance;
			ShaderStorageTexture OutputSampleCount;
			ShaderStaticSampler LinearSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderReadBuffer GPUSceneScene;
			ShaderPushConstants<RayTracedReflectionsTemporalParameters> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Current, 0, 0);
			Binder.Bind(Params.History, 0, 1);
			Binder.Bind(Params.Velocity, 0, 2);
			Binder.Bind(Params.Depth, 0, 3);
			Binder.Bind(Params.DepthHistory, 0, 4);
			Binder.Bind(Params.HistorySampleCount, 0, 5);
			Binder.Bind(Params.Rays, 0, 6);
			Binder.Bind(Params.ResolvedHitDistance, 0, 7);
			Binder.Bind(Params.Normal, 0, 8);
			Binder.Bind(Params.RoughnessMetallic, 0, 9);
			Binder.Bind(Params.NormalHistory, 0, 10);
			Binder.Bind(Params.RoughnessMetallicHistory, 0, 11);
			Binder.Bind(Params.OutputRadiance, 0, 12);
			Binder.Bind(Params.OutputSampleCount, 0, 13);
			Binder.Bind(Params.LinearSampler, 0, 14);
			Binder.Bind(Params.GPUSceneScene, 0, 15);
			Binder.Bind(Params.Constants);
		}
	};

	static Buffer*& ReflectionDenoiserConstantBuffer()
	{
		static Buffer* CB = nullptr;
		return CB;
	}

	static void UploadReflectionDenoiserConstants(RenderGraph& Graph, const RenderView& View)
	{
		Buffer*& CB = ReflectionDenoiserConstantBuffer();
		static Buffer* UploadBuffers[MaxFramesInFlight]{ nullptr };

		if (CB == nullptr)
		{
			CB = Graph.Device->CreateBuffer(BufferDesc(sizeof(RTReflectionDenoiserConstants), BufferType::Constant), nullptr);
			for (int i = 0; i < MaxFramesInFlight; i++)
			{
				BufferDesc UploadDesc(sizeof(RTReflectionDenoiserConstants), BufferType::Constant);
				UploadDesc.HostVisible = true;
				UploadBuffers[i] = Graph.Device->CreateBuffer(UploadDesc, nullptr);
			}
		}

		RTReflectionDenoiserConstants Constants;
		Constants.InvViewProjection = View.CameraCur.GetViewProjection().GetInverted();
		Constants.Projection = View.CameraCur.GetProjectionMatrix();
		Constants.InvProjection = View.CameraCur.GetProjectionMatrix().GetInverted();
		Constants.View = View.CameraCur.GetViewMatrix();
		Constants.InvView = View.CameraCur.GetViewMatrix().GetInverted();
		Constants.PrevViewProjection = View.CameraPrev.GetViewProjection();
		Constants.BufferDimensions[0] = (u32)View.RenderSize.X;
		Constants.BufferDimensions[1] = (u32)View.RenderSize.Y;
		Constants.InvBufferDimensions[0] = 1.0f / (float)View.RenderSize.X;
		Constants.InvBufferDimensions[1] = 1.0f / (float)View.RenderSize.Y;
		Constants.TemporalStabilityFactor = 0.7f;
		Constants.DepthBufferThickness = 0.015f;
		const float PerceptualRoughnessThreshold = CVar_MaxRoughness.GetValue();
		Constants.RoughnessThreshold = PerceptualRoughnessThreshold * PerceptualRoughnessThreshold;
		Constants.VarianceThreshold = 0.0f;
		Constants.FrameIndex = (u32)GFrameNumber;
		Constants.MaxTraversalIntersections = 128;
		Constants.MinTraversalOccupancy = 4;
		Constants.MostDetailedMip = 0;
		Constants.SamplesPerQuad = 1;
		Constants.TemporalVarianceGuidedTracingEnabled = 1;

		const u32 BufferId = (u32)(GFrameNumber % MaxFramesInFlight);
		void* Map = Graph.Device->MapBuffer(UploadBuffers[BufferId]);
		memcpy(Map, &Constants, sizeof(Constants));
		Graph.Device->UnmapBuffer(UploadBuffers[BufferId]);

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Graph.AddPass("RTReflDenoiserUploadConstants", RenderGraphPassType::Compute, Parameters, Dependencies, [BufferId, CB](RenderGraphContext& Context)
		{
			Context.CommandBuffer->CopyBuffer(UploadBuffers[BufferId], CB, 0, 0, sizeof(RTReflectionDenoiserConstants));
		});
	}

	static Buffer* GetReflectionDenoiserConstantsBuffer(RenderGraph& Graph, const RenderView& View)
	{
		UploadReflectionDenoiserConstants(Graph, View);
		return ReflectionDenoiserConstantBuffer();
	}

	static RenderGraphTextureRef DenoiseReflectionsFFX(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, RenderGraphTextureRef InputRadiance)
	{
		RENDER_GRAPH_SCOPED_MARKER(Graph, "RayTracedReflections FFX Denoise");

		const iVector2 Size = View.RenderSize;
		Buffer* ConstantsBuffer = GetReflectionDenoiserConstantsBuffer(Graph, View);

		TextureDesc2 RadianceDesc{
			.Usage = TextureUsage::StorageSampled,
			.Width = (u32)Size.X,
			.Height = (u32)Size.Y,
			.Format = TextureFormat::RGBA16F,
		};

		TextureDesc2 SingleChannelDesc = RadianceDesc;
		SingleChannelDesc.Format = TextureFormat::R16F;

		TextureDesc2 AverageRadianceDesc = RadianceDesc;
		AverageRadianceDesc.Width = (u32)((Size.X + 7) / 8);
		AverageRadianceDesc.Height = (u32)((Size.Y + 7) / 8);

		Graph.CreateHistoryTexture(&Textures.History.RTReflectionsRadiance, RadianceDesc, "RTRefl FFX Radiance History");
		Graph.CreateHistoryTexture(&Textures.History.RTReflectionsAverageRadiance, AverageRadianceDesc, "RTRefl FFX AverageRadiance History");
		Graph.CreateHistoryTexture(&Textures.History.RTReflectionsVariance, SingleChannelDesc, "RTRefl FFX Variance History");
		Graph.CreateHistoryTexture(&Textures.History.RTReflectionsSampleCount, SingleChannelDesc, "RTRefl FFX SampleCount History");

		RenderGraphTextureRef ReprojectedRadiance = Graph.CreateTexture(RadianceDesc, "RTRefl FFX ReprojectedRadiance");
		RenderGraphTextureRef AverageRadiance = Graph.CreateTexture(AverageRadianceDesc, "RTRefl FFX AverageRadiance");
		RenderGraphTextureRef Radiance1 = Graph.CreateTexture(RadianceDesc, "RTRefl FFX Radiance1");
		RenderGraphTextureRef Radiance2 = Graph.CreateTexture(RadianceDesc, "RTRefl FFX Radiance2");
		RenderGraphTextureRef Variance1 = Graph.CreateTexture(SingleChannelDesc, "RTRefl FFX Variance1");
		RenderGraphTextureRef Variance2 = Graph.CreateTexture(SingleChannelDesc, "RTRefl FFX Variance2");
		RenderGraphTextureRef SampleCount1 = Graph.CreateTexture(SingleChannelDesc, "RTRefl FFX SampleCount1");
		RenderGraphTextureRef SampleCount2 = Graph.CreateTexture(SingleChannelDesc, "RTRefl FFX SampleCount2");

		// Reproject history and estimate variance / average radiance.
		{
			ReflectionDenoiserReprojectShader::Parameters ReprojectParams;
			ReprojectParams.Constants = ConstantsBuffer;
			ReprojectParams.Depth = Textures.GBufferDS;
			ReprojectParams.Roughness = Textures.GBufferRM;
			ReprojectParams.Normal = Textures.GBufferNormal;
			ReprojectParams.DepthHistory = Textures.History.Depth;
			ReprojectParams.RoughnessHistory = Textures.History.RoughnessMetallic;
			ReprojectParams.NormalHistory = Textures.History.Normals;
			ReprojectParams.InputRadiance = InputRadiance;
			ReprojectParams.RadianceHistory = Textures.History.RTReflectionsRadiance;
			ReprojectParams.Velocity = Textures.Velocity;
			ReprojectParams.AverageRadianceHistory = Textures.History.RTReflectionsAverageRadiance;
			ReprojectParams.VarianceHistory = Textures.History.RTReflectionsVariance;
			ReprojectParams.SampleCountHistory = Textures.History.RTReflectionsSampleCount;
			ReprojectParams.ReprojectedRadiance = ReprojectedRadiance;
			ReprojectParams.AverageRadiance = AverageRadiance;
			ReprojectParams.Variance = Variance1;
			ReprojectParams.SampleCount = SampleCount1;

			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<ReflectionDenoiserReprojectShader>(ReprojectParams);

			Graph.AddPass("RTRefl FFX Reproject", RenderGraphPassType::Compute, Parameters, Dependencies, [ReprojectParams, Size](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflectionsDenoise, Context);

				ComputePipeline* Pipeline = GetComputePipeline<ReflectionDenoiserReprojectShader>(Context, ReflectionDenoiserReprojectShader::Permutation {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<ReflectionDenoiserReprojectShader>(Pipeline, ReprojectParams);
				Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { Size, 1 });
			});
		}

		// Variance-guided spatial prefilter.
		{
			ReflectionDenoiserPrefilterShader::Parameters PrefilterParams;
			PrefilterParams.Constants = ConstantsBuffer;
			PrefilterParams.Depth = Textures.GBufferDS;
			PrefilterParams.Roughness = Textures.GBufferRM;
			PrefilterParams.Normal = Textures.GBufferNormal;
			PrefilterParams.AverageRadiance = AverageRadiance;
			PrefilterParams.InputRadiance = InputRadiance;
			PrefilterParams.Variance = Variance1;
			PrefilterParams.SampleCount = SampleCount1;
			PrefilterParams.OutputRadiance = Radiance1;
			PrefilterParams.OutputVariance = Variance2;
			PrefilterParams.OutputSampleCount = SampleCount2;

			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<ReflectionDenoiserPrefilterShader>(PrefilterParams);

			Graph.AddPass("RTRefl FFX Prefilter", RenderGraphPassType::Compute, Parameters, Dependencies, [PrefilterParams, Size](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflectionsDenoise, Context);

				ComputePipeline* Pipeline = GetComputePipeline<ReflectionDenoiserPrefilterShader>(Context, ReflectionDenoiserPrefilterShader::Permutation {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<ReflectionDenoiserPrefilterShader>(Pipeline, PrefilterParams);
				Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { Size, 1 });
			});
		}

		// Temporal resolve and history output.
		{
			ReflectionDenoiserResolveTemporalShader::Parameters ResolveParams;
			ResolveParams.Constants = ConstantsBuffer;
			ResolveParams.Roughness = Textures.GBufferRM;
			ResolveParams.AverageRadiance = AverageRadiance;
			ResolveParams.InputRadiance = Radiance1;
			ResolveParams.ReprojectedRadiance = ReprojectedRadiance;
			ResolveParams.Variance = Variance2;
			ResolveParams.SampleCount = SampleCount2;
			ResolveParams.OutputRadiance = Radiance2;
			ResolveParams.OutputVariance = Variance1;
			ResolveParams.OutputSampleCount = SampleCount1;

			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<ReflectionDenoiserResolveTemporalShader>(ResolveParams);

			Graph.AddPass("RTRefl FFX ResolveTemporal", RenderGraphPassType::Compute, Parameters, Dependencies, [ResolveParams, Size](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflectionsDenoise, Context);

				ComputePipeline* Pipeline = GetComputePipeline<ReflectionDenoiserResolveTemporalShader>(Context, ReflectionDenoiserResolveTemporalShader::Permutation {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<ReflectionDenoiserResolveTemporalShader>(Pipeline, ResolveParams);
				Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { Size, 1 });
			});
		}

		Graph.ExtractTexture(Radiance2, &Textures.History.RTReflectionsRadiance);
		Graph.ExtractTexture(AverageRadiance, &Textures.History.RTReflectionsAverageRadiance);
		Graph.ExtractTexture(Variance1, &Textures.History.RTReflectionsVariance);
		Graph.ExtractTexture(SampleCount1, &Textures.History.RTReflectionsSampleCount);

		return Radiance2;
	}

	// TODO: downscale reflections resolution and then upscale (FSR1?)
	// TODO: sample lighting in reflection (requires volumetric GI)
	void RayTracedReflectionsPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext, const RayTracingIrradianceVolumes::Prepared& IrradianceVolumes)
	{
		RENDER_GRAPH_SCOPED_MARKER(Graph, "RayTracedReflections");

		iVector2 TraceSize = View.RenderSize;

		TextureDesc2 Desc{
			.Usage = TextureUsage::StorageSampled,
			.Width = (u32)TraceSize.X,
			.Height = (u32)TraceSize.Y,
			.Format = TextureFormat::RGBA16F,
		};

		TextureDesc2 SingleChannelDesc = Desc;
		SingleChannelDesc.Format = TextureFormat::R16F;

		const bool Resolve = View.DeferredSettings.RTReflectionsResolve;
		const bool Temporal = View.DeferredSettings.RTReflectionsTemporal;
		const bool UseRadianceCache = View.DeferredSettings.RTReflectionsUseRadianceCache;
		const int TemporalMaxSamples = View.DeferredSettings.RTReflectionsTemporalMaxSamples;

		RenderGraphTextureRef RTReflectionRadiance = Graph.CreateTexture(Desc, "RTReflRadiance");
		RenderGraphTextureRef RTReflectionRays = Graph.CreateTexture(Desc, "RTReflRays");
		RenderGraphTextureRef RTReflectionRayPdf = Graph.CreateTexture(SingleChannelDesc, "RTReflRayPdf");

		{
			RenderPassParameters Parameters;

			RayTracedReflectionsShader::Parameters TraceParams;
			TraceParams.Scene.UseCombinedSampler = false;
			TraceParams.Output = RTReflectionRadiance;
			TraceParams.ResultDirectionDistance = RTReflectionRays;
			TraceParams.ResultRayPdf = RTReflectionRayPdf;
			TraceParams.GBufferAlbedo = Textures.GBufferAlbedo;
			TraceParams.GBufferNormals = Textures.GBufferNormal;
			TraceParams.GBufferRoughnessMetallic = Textures.GBufferRM;
			TraceParams.GBufferDepth = Textures.GBufferDS;
			TraceParams.IrradianceVolumeConstants = IrradianceVolumes.Constants;
			TraceParams.IrradianceProbeBuffers = IrradianceVolumes.ProbeBuffers;
			TraceParams.Constants.Value = {
				.CameraPosition = Vector4(View.CameraCur.Pos, 1),
				.MaxRoughness = CVar_MaxRoughness.GetValue(),
				.Random = (u32)GFrameNumber,
				.UseRadianceCache = (u32)UseRadianceCache,
			};

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<RayTracedReflectionsShader>(TraceParams);

			Graph.AddPass("RayTraceReflections", RenderGraphPassType::Compute, Parameters, Dependencies, [TraceParams, View](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflections, Context);

				RayTracedReflectionsShader::Parameters Parameters = TraceParams;
				Parameters.AccelerationStructure = Context.Scene->TLAS;

				RayTracingPipeline* Pipeline = GetRayTracingPipeline<RayTracedReflectionsShader>(Context, RayTracedReflectionsShader::Permutation {}, RayTracedReflectionsShader::PipelinePermutation {});
				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindRayTracingParameters<RayTracedReflectionsShader>(Pipeline, Parameters);
				Context.CommandBuffer->TraceRays(Pipeline, View.RenderSize.X, View.RenderSize.Y, 1);
			});
		}

		if (CVar_Denoiser.GetValue() == 1)
		{
			Textures.RTReflections = DenoiseReflectionsFFX(Graph, View, Textures, RTReflectionRadiance);
			return;
		}

		RenderGraphTextureRef ReflectionsResult = RTReflectionRadiance;
		RenderGraphTextureRef ReflectionsHitDistance = -1;

		// BRDF/PDF-based spatial reuse pass. Temporal also needs the resolved hit distance for hit-point reprojection.
		if (Resolve || Temporal)
		{
			RenderGraphTextureRef RTResolveResult = Graph.CreateTexture(Desc, "RTReflResolved");
			RenderGraphTextureRef RTResolveHitDistance = Graph.CreateTexture(SingleChannelDesc, "RTReflResolvedHitDistance");

			ReflectionsResolveShader::Parameters ResolveParams;
			ResolveParams.Radiance = RTReflectionRadiance;
			ResolveParams.Rays = RTReflectionRays;
			ResolveParams.RayPdf = RTReflectionRayPdf;
			ResolveParams.GBufferAlbedo = Textures.GBufferAlbedo;
			ResolveParams.GBufferNormal = Textures.GBufferNormal;
			ResolveParams.GBufferRM = Textures.GBufferRM;
			ResolveParams.GBufferDepth = Textures.GBufferDS;
			ResolveParams.Output = RTResolveResult;
			ResolveParams.OutputHitDistance = RTResolveHitDistance;
			ResolveParams.Constants.Value = {
				.ImageSize = TraceSize,
				._Padding = iVector2(0, 0),
			};

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<ReflectionsResolveShader>(ResolveParams);

			Graph.AddPass("RTReflResolve", RenderGraphPassType::Compute, Parameters, Dependencies, [ResolveParams, TraceSize](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflectionsDenoise, Context);

				ReflectionsResolveShader::Parameters Parameters = ResolveParams;
				Parameters.GPUSceneScene = Context.Scene->SceneBuffer;

				ComputePipeline* Pipeline = GetComputePipeline<ReflectionsResolveShader>(Context, ReflectionsResolveShader::Permutation {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<ReflectionsResolveShader>(Pipeline, Parameters);
				Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { TraceSize, 1 });
			});

			if (Resolve)
			{
				ReflectionsResult = RTResolveResult;
			}
			ReflectionsHitDistance = RTResolveHitDistance;
		}

		// Minimal temporal resolve (history + reprojection + neighborhood clamp + sample count)
		if (Temporal)
		{
			Graph.CreateHistoryTexture(&Textures.History.RTReflectionsRadiance, Desc, "RTReflTemporalHistory");
			Graph.CreateHistoryTexture(&Textures.History.RTReflectionsSampleCount, SingleChannelDesc, "RTReflTemporalSampleCountHistory");

			RenderGraphTextureRef RTTemporalResult = Graph.CreateTexture(Desc, "RTReflTemporal");
			RenderGraphTextureRef RTTemporalSampleCount = Graph.CreateTexture(SingleChannelDesc, "RTReflTemporalSampleCount");

			Texture2* ReflectionHistory = Textures.History.RTReflectionsRadiance;
			Texture2* ReflectionSampleCountHistory = Textures.History.RTReflectionsSampleCount;

			ReflectionsTemporalShader::Parameters TemporalParams;
			TemporalParams.Current = ReflectionsResult;
			TemporalParams.History = ReflectionHistory;
			TemporalParams.Velocity = Textures.Velocity;
			TemporalParams.Depth = Textures.GBufferDS;
			TemporalParams.DepthHistory = Textures.History.Depth;
			TemporalParams.HistorySampleCount = ReflectionSampleCountHistory;
			TemporalParams.Rays = RTReflectionRays;
			TemporalParams.ResolvedHitDistance = ReflectionsHitDistance;
			TemporalParams.Normal = Textures.GBufferNormal;
			TemporalParams.RoughnessMetallic = Textures.GBufferRM;
			TemporalParams.NormalHistory = Textures.History.Normals;
			TemporalParams.RoughnessMetallicHistory = Textures.History.RoughnessMetallic;
			TemporalParams.OutputRadiance = RTTemporalResult;
			TemporalParams.OutputSampleCount = RTTemporalSampleCount;
			TemporalParams.Constants.Value = {
				.Size = TraceSize,
				.MaxSamples = TemporalMaxSamples,
				._Padding = 0,
			};

			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<ReflectionsTemporalShader>(TemporalParams);

			Graph.AddPass("RTReflTemporal", RenderGraphPassType::Compute, Parameters, Dependencies, [TemporalParams, TraceSize](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflectionsDenoise, Context);

				ReflectionsTemporalShader::Parameters Parameters = TemporalParams;
				Parameters.GPUSceneScene = Context.Scene->SceneBuffer;

				ComputePipeline* Pipeline = GetComputePipeline<ReflectionsTemporalShader>(Context, ReflectionsTemporalShader::Permutation {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<ReflectionsTemporalShader>(Pipeline, Parameters);
				Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { TraceSize, 1 });
			});

			Graph.ExtractTexture(RTTemporalResult, &Textures.History.RTReflectionsRadiance);
			Graph.ExtractTexture(RTTemporalSampleCount, &Textures.History.RTReflectionsSampleCount);
			ReflectionsResult = RTTemporalResult;
		}

		Textures.RTReflections = ReflectionsResult;
	}

}
