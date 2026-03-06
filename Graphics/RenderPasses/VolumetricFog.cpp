#include "Core/Core.h"
#include "Graphics/Core/Pipelines.h"
#include "Graphics/Core/Texture.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"
#include "Profiling/Profiling.h"

namespace Columbus
{
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterVolumetricFogInject);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterVolumetricFogComposite);

	IMPLEMENT_GPU_PROFILING_COUNTER("Volumetric Fog Inject", "RenderGraphGPU", GpuCounterVolumetricFogInject);
	IMPLEMENT_GPU_PROFILING_COUNTER("Volumetric Fog Composite", "RenderGraphGPU", GpuCounterVolumetricFogComposite);

	struct VolumetricFogInjectParameters
	{
		u32 FrameIndex;
		u32 MaxLights;
		u32 ShadowSamples;
		u32 Flags;
		float ShadowJitter;
		float FroxelJitter;
		float TemporalBlendFactor;
		float HistoryClip;
	};

	struct VolumetricFogCompositeParameters
	{
		iVector2 RenderSize;
		u32 FrameIndex;
		u32 SampleFilter;
		float SkyTransmittance;
		float FroxelJitter;
		float _pad[2];
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
			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Write(FroxelScattering, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass("VolumetricFogInject", RenderGraphPassType::Compute, Parameters, Dependencies, [FroxelScattering, FroxelHistoryTexture, FroxelResolution, MaxLights, ShadowSamples, ShadowJitter, FroxelJitter, bHasFroxelHistory, TemporalBlend, HistoryClip](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterVolumetricFogInject, Context);

				static RayTracingPipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					RayTracingPipelineDesc Desc;
					Desc.Name = "VolumetricFogInject";
					Desc.MaxRecursionDepth = 1;
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/VolumetricFog/Inject.csd");
					Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
				}

				static VkDescriptorSet DescriptorSets[MaxFramesInFlight]{ NULL };
				if (DescriptorSets[Context.RenderData.CurrentPerFrameData] == NULL)
				{
					DescriptorSets[Context.RenderData.CurrentPerFrameData] = Context.Device->CreateDescriptorSet(Pipeline, 2);
				}

				auto Set = DescriptorSets[Context.RenderData.CurrentPerFrameData];
				Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.Scene->TLAS);
				Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(FroxelScattering).get());
				Context.Device->UpdateDescriptorSet(Set, 2, 0, FroxelHistoryTexture, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());

				VolumetricFogInjectParameters Params{
					.FrameIndex = (u32)GFrameNumber,
					.MaxLights = (u32)MaxLights,
					.ShadowSamples = (u32)ShadowSamples,
					.Flags = bHasFroxelHistory ? 1u : 0u,
					.ShadowJitter = ShadowJitter,
					.FroxelJitter = FroxelJitter,
					.TemporalBlendFactor = TemporalBlend,
					.HistoryClip = HistoryClip,
				};

				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindGPUScene(Pipeline, false);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &Set);
				Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Params), &Params);
				Context.CommandBuffer->TraceRays(Pipeline, FroxelResolution.X, FroxelResolution.Y, FroxelResolution.Z);
			});
		}

		RenderGraphTextureRef FoggedScene = Graph.CreateTexture(SceneDesc, "VolumetricFogComposite");
		Textures.VolumetricFog = Graph.CreateTexture(FogOnlyDesc, "VolumetricFog");

		{
			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(SceneTexture, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(FroxelScattering, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(FoggedScene, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Textures.VolumetricFog, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("VolumetricFogComposite", RenderGraphPassType::Compute, Parameters, Dependencies, [SceneTexture, FoggedScene, FroxelScattering, Textures, View, SampleFilter, SkyTransmittance, FroxelJitter](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterVolumetricFogComposite, Context);

				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "VolumetricFogComposite";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/VolumetricFog/Composite.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto Set = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(SceneTexture).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.GetRenderGraphTexture(FroxelScattering).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 4, 0, Context.GetRenderGraphTexture(FoggedScene).get());
				Context.Device->UpdateDescriptorSet(Set, 5, 0, Context.GetRenderGraphTexture(Textures.VolumetricFog).get());
				Context.Device->UpdateDescriptorSet(Set, 6, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());
				Context.Device->UpdateDescriptorSet(Set, 7, 0, Context.Scene->SceneBuffer);

				VolumetricFogCompositeParameters Params{
					.RenderSize = View.RenderSize,
					.FrameIndex = (u32)GFrameNumber,
					.SampleFilter = (u32)SampleFilter,
					.SkyTransmittance = SkyTransmittance,
					.FroxelJitter = FroxelJitter,
					._pad = { 0.0f, 0.0f },
				};

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
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
