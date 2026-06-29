#include "Core/Core.h"
#include "Graphics/Core/Texture.h"
#include "Graphics/ShaderCache.h"
#include "RenderPasses.h"
#include "Profiling/Profiling.h"

namespace Columbus
{
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterVolumetricFogInject);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterVolumetricFogComposite);

	IMPLEMENT_GPU_PROFILING_COUNTER("Volumetric Fog Inject", "RenderGraphGPU", GpuCounterVolumetricFogInject);
	IMPLEMENT_GPU_PROFILING_COUNTER("Volumetric Fog Composite", "RenderGraphGPU", GpuCounterVolumetricFogComposite);

	struct VolumetricFogInjectParameters
	{
		u32 FrameIndex = 0;
		u32 MaxLights = 0;
		u32 ShadowSamples = 0;
		u32 Flags = 0;
		float ShadowJitter = 0.0f;
		float FroxelJitter = 0.0f;
		float TemporalBlendFactor = 0.0f;
		float HistoryClip = 0.0f;
	};

	struct VolumetricFogCompositeParameters
	{
		iVector2 RenderSize{};
		u32 FrameIndex = 0;
		u32 SampleFilter = 0;
		float SkyTransmittance = 1.0f;
		float FroxelJitter = 0.0f;
		float _pad[2]{};
	};

	struct VolumetricFogInjectShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/VolumetricFog/Inject.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct PipelinePermutation
		{
			u32 MaxRecursionDepth = 1;
		};

		static RayTracingPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			RayTracingPipelineDesc Desc {};
			Desc.Name = "VolumetricFogInject";
			Desc.MaxRecursionDepth = Permutation.MaxRecursionDepth;
			return Desc;
		}

		struct Parameters
		{
			ShaderGPUScene Scene;
			ShaderAccelerationStructure AccelerationStructure;
			ShaderStorageTexture OutputFroxels;
			ShaderSampledTexture HistoryFroxels;
			ShaderStaticSampler LinearSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderPushConstants<VolumetricFogInjectParameters> Constants { {}, ShaderType::Raygen };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.AccelerationStructure, 2, 0);
			Binder.Bind(Params.OutputFroxels, 2, 1);
			Binder.Bind(Params.HistoryFroxels, 2, 2);
			Binder.Bind(Params.LinearSampler, 2, 3);
			Binder.Bind(Params.Constants);
		}
	};

	struct VolumetricFogCompositeShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/VolumetricFog/Composite.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct Parameters
		{
			ShaderSampledTexture SceneColor;
			ShaderSampledTexture SceneDepth { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture FroxelScattering;
			ShaderStorageTexture OutputColor;
			ShaderStorageTexture OutputFog;
			ShaderStaticSampler LinearSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderReadBuffer GPUSceneScene;
			ShaderPushConstants<VolumetricFogCompositeParameters> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.SceneColor, 0, 0);
			Binder.Bind(Params.SceneDepth, 0, 1);
			Binder.Bind(Params.FroxelScattering, 0, 2);
			Binder.Bind(Params.OutputColor, 0, 3);
			Binder.Bind(Params.OutputFog, 0, 4);
			Binder.Bind(Params.LinearSampler, 0, 5);
			Binder.Bind(Params.GPUSceneScene, 0, 6);
			Binder.Bind(Params.Constants);
		}
	};

	RenderGraphTextureRef RenderVolumetricFog(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, RenderGraphTextureRef SceneTexture)
	{
		RENDER_GRAPH_SCOPED_MARKER(Graph, "VolumetricFog");

		const HVolumetricFogSettings& Fog = View.EffectsSettings.VolumetricFog;
		const bool bFogEnabled = Fog.EnableVolumetricFog;
		const int SampleFilter = Math::Clamp(Fog.SampleFilter, 0, 3);
		const float TemporalBlend = Math::Clamp(Fog.TemporalBlend, 0.0f, 0.95f);
		const float SkyTransmittance = Math::Clamp(Fog.SkyTransmittance, 0.0f, 1.0f);
		const int ShadowSamples = Math::Clamp(Fog.ShadowSamples, 1, 8);
		const float ShadowJitter = Math::Clamp(Fog.ShadowJitter, 0.0f, 4.0f);
		const float FroxelJitter = Math::Clamp(Fog.FroxelJitter, 0.0f, 2.0f);
		const float HistoryClip = Math::Clamp(Fog.HistoryClip, 0.02f, 1.0f);
		const int MaxLights = Math::Clamp(Fog.MaxLights, 1, 1024);

		static int PrevShadowSamples = -1;
		static float PrevShadowJitter = -1.0f;
		static float PrevFroxelJitter = -1.0f;
		static float PrevTemporalBlend = -1.0f;
		static float PrevHistoryClip = -1.0f;
		static iVector3 PrevFroxelResolution = iVector3(0, 0, 0);
		static bool PrevFogEnabled = false;

		const bool bHistorySettingsChanged =
			(PrevShadowSamples != ShadowSamples) ||
			(Math::Abs(PrevShadowJitter - ShadowJitter) > 1e-4f) ||
			(Math::Abs(PrevFroxelJitter - FroxelJitter) > 1e-4f) ||
			(Math::Abs(PrevTemporalBlend - TemporalBlend) > 1e-4f) ||
			(Math::Abs(PrevHistoryClip - HistoryClip) > 1e-4f) ||
			(!PrevFogEnabled);

		TextureDesc2 SceneDesc = Graph.GetTextureDesc(SceneTexture);
		TextureDesc2 FogOnlyDesc = SceneDesc;
		FogOnlyDesc.Usage = TextureUsage::StorageSampled;
		FogOnlyDesc.Format = TextureFormat::RGBA16F;
		FogOnlyDesc.AddressU = TextureAddressMode::Repeat;
		FogOnlyDesc.AddressV = TextureAddressMode::Repeat;
		FogOnlyDesc.AddressW = TextureAddressMode::Repeat;
		FogOnlyDesc.MinFilter = TextureFilter2::Linear;
		FogOnlyDesc.MagFilter = TextureFilter2::Linear;
		FogOnlyDesc.MipFilter = TextureFilter2::Linear;

		if (!bFogEnabled)
		{
			PrevFogEnabled = false;
			Textures.VolumetricFog = Graph.CreateTexture(FogOnlyDesc, "VolumetricFog");
			ShaderMemsetTexture(Graph, Textures.VolumetricFog, Vector4(0, 0, 0, 0));
			return SceneTexture;
		}

		const int FroxelPixelSize = Math::Clamp(Fog.FroxelPixelSize, 4, 64);
		const int FroxelSlices = Math::Clamp(Fog.FroxelSlices, 8, 256);
		const iVector3 FroxelResolution = iVector3(
			(View.RenderSize.X + FroxelPixelSize - 1) / FroxelPixelSize,
			(View.RenderSize.Y + FroxelPixelSize - 1) / FroxelPixelSize,
			FroxelSlices);
		const bool bFroxelResolutionChanged = PrevFroxelResolution != FroxelResolution;

		TextureDesc2 FroxelDesc;
		FroxelDesc.Type = TextureType::Texture3D;
		FroxelDesc.Usage = TextureUsage::StorageSampled;
		FroxelDesc.Width = (u32)FroxelResolution.X;
		FroxelDesc.Height = (u32)FroxelResolution.Y;
		FroxelDesc.Depth = (u32)FroxelResolution.Z;
		FroxelDesc.Format = TextureFormat::RGBA16F;
		FroxelDesc.AddressU = TextureAddressMode::ClampToEdge;
		FroxelDesc.AddressV = TextureAddressMode::ClampToEdge;
		FroxelDesc.AddressW = TextureAddressMode::ClampToEdge;
		FroxelDesc.MinFilter = TextureFilter2::Linear;
		FroxelDesc.MagFilter = TextureFilter2::Linear;
		FroxelDesc.MipFilter = TextureFilter2::Linear;

		const bool bHistoryInvalidated = Graph.CreateHistoryTexture(&Textures.History.VolumetricFogFroxels, FroxelDesc, "VolumetricFog Froxels History");
		Texture2* FroxelHistoryTexture = Textures.History.VolumetricFogFroxels;
		const bool bHasFroxelHistory = (GFrameNumber > 1) && !bHistoryInvalidated && !bFroxelResolutionChanged && !bHistorySettingsChanged && (TemporalBlend > 0.0f);

		RenderGraphTextureRef FroxelScattering = Graph.CreateTexture(FroxelDesc, "VolumetricFogFroxels");

		{
			VolumetricFogInjectShader::Parameters InjectParams;
			InjectParams.Scene.UseCombinedSampler = false;
			InjectParams.OutputFroxels = FroxelScattering;
			InjectParams.HistoryFroxels = FroxelHistoryTexture;
			InjectParams.Constants.Value = {
				.FrameIndex = (u32)GFrameNumber,
				.MaxLights = (u32)MaxLights,
				.ShadowSamples = (u32)ShadowSamples,
				.Flags = bHasFroxelHistory ? 1u : 0u,
				.ShadowJitter = ShadowJitter,
				.FroxelJitter = FroxelJitter,
				.TemporalBlendFactor = TemporalBlend,
				.HistoryClip = HistoryClip,
			};

			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<VolumetricFogInjectShader>(InjectParams);

			Graph.AddPass("VolumetricFogInject", RenderGraphPassType::Compute, Parameters, Dependencies, [InjectParams, FroxelResolution](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterVolumetricFogInject, Context);

				VolumetricFogInjectShader::Parameters Parameters = InjectParams;
				Parameters.AccelerationStructure = Context.Scene->TLAS;

				RayTracingPipeline* Pipeline = GetRayTracingPipeline<VolumetricFogInjectShader>(Context, VolumetricFogInjectShader::Permutation {}, VolumetricFogInjectShader::PipelinePermutation {});
				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindRayTracingParameters<VolumetricFogInjectShader>(Pipeline, Parameters);
				Context.CommandBuffer->TraceRays(Pipeline, FroxelResolution.X, FroxelResolution.Y, FroxelResolution.Z);
			});
		}

		RenderGraphTextureRef FoggedScene = Graph.CreateTexture(SceneDesc, "VolumetricFogComposite");
		Textures.VolumetricFog = Graph.CreateTexture(FogOnlyDesc, "VolumetricFog");

		{
			VolumetricFogCompositeShader::Parameters CompositeParams;
			CompositeParams.SceneColor = SceneTexture;
			CompositeParams.SceneDepth = Textures.GBufferDS;
			CompositeParams.FroxelScattering = FroxelScattering;
			CompositeParams.OutputColor = FoggedScene;
			CompositeParams.OutputFog = Textures.VolumetricFog;
			CompositeParams.Constants.Value = {
				.RenderSize = View.RenderSize,
				.FrameIndex = (u32)GFrameNumber,
				.SampleFilter = (u32)SampleFilter,
				.SkyTransmittance = SkyTransmittance,
				.FroxelJitter = FroxelJitter,
				._pad = { 0.0f, 0.0f },
			};

			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<VolumetricFogCompositeShader>(CompositeParams);

			Graph.AddPass("VolumetricFogComposite", RenderGraphPassType::Compute, Parameters, Dependencies, [CompositeParams, View](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterVolumetricFogComposite, Context);

				VolumetricFogCompositeShader::Parameters Parameters = CompositeParams;
				Parameters.GPUSceneScene = Context.Scene->SceneBuffer;

				ComputePipeline* Pipeline = GetComputePipeline<VolumetricFogCompositeShader>(Context, VolumetricFogCompositeShader::Permutation {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<VolumetricFogCompositeShader>(Pipeline, Parameters);
				Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { View.RenderSize, 1 });
			});
		}

		Graph.ExtractTexture(FroxelScattering, &Textures.History.VolumetricFogFroxels);

		PrevShadowSamples = ShadowSamples;
		PrevShadowJitter = ShadowJitter;
		PrevFroxelJitter = FroxelJitter;
		PrevTemporalBlend = TemporalBlend;
		PrevHistoryClip = HistoryClip;
		PrevFroxelResolution = FroxelResolution;
		PrevFogEnabled = true;

		return FoggedScene;
	}

}
