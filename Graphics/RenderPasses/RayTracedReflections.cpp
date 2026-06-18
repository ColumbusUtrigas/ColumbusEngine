#include "RenderPasses.h"
#include "RayTracingIrradianceVolumes.h"
#include "Core/Core.h"
#include "Core/CVar.h"

#include <Lib/imgui/imgui.h>

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
		Vector4 CameraPosition;
		float MaxRoughness;
		u32 Random;
		u32 UseRadianceCache;
	};

	struct RayTracedReflectionsResolveParameters
	{
		iVector2 ImageSize;
		iVector2 _Padding;
	};

	struct RayTracedReflectionsTemporalParameters
	{
		iVector2 Size;
		int MaxSamples;
		int _Padding;
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
			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(InputRadiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.Velocity, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(ReprojectedRadiance, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(AverageRadiance, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Variance1, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(SampleCount1, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("RTRefl FFX Reproject", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, InputRadiance, ReprojectedRadiance, AverageRadiance, Variance1, SampleCount1, ConstantsBuffer, Size](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflectionsDenoise, Context);

				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "RTRefl_FFX_Reproject";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/DenoiserReflection/Reproject.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto CBSet = Context.GetDescriptorSet(Pipeline, 0);
				auto Set = Context.GetDescriptorSet(Pipeline, 1);
				Context.Device->UpdateDescriptorSet(CBSet, 0, 0, ConstantsBuffer);

				Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 3, 0, Textures.History.Depth, TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 4, 0, Textures.History.RoughnessMetallic, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 5, 0, Textures.History.Normals, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 6, 0, Context.GetRenderGraphTexture(InputRadiance).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 7, 0, Textures.History.RTReflectionsRadiance, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 8, 0, Context.GetRenderGraphTexture(Textures.Velocity).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 9, 0, Textures.History.RTReflectionsAverageRadiance, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 10, 0, Textures.History.RTReflectionsVariance, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 11, 0, Textures.History.RTReflectionsSampleCount, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 12, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());
				Context.Device->UpdateDescriptorSet(Set, 13, 0, Context.GetRenderGraphTexture(ReprojectedRadiance).get());
				Context.Device->UpdateDescriptorSet(Set, 14, 0, Context.GetRenderGraphTexture(AverageRadiance).get());
				Context.Device->UpdateDescriptorSet(Set, 15, 0, Context.GetRenderGraphTexture(Variance1).get());
				Context.Device->UpdateDescriptorSet(Set, 16, 0, Context.GetRenderGraphTexture(SampleCount1).get());

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &CBSet);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 1, 1, &Set);
				Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { Size, 1 });
			});
		}

		// Variance-guided spatial prefilter.
		{
			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(AverageRadiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(InputRadiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Variance1, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(SampleCount1, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Radiance1, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Variance2, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(SampleCount2, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("RTRefl FFX Prefilter", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, InputRadiance, AverageRadiance, Variance1, SampleCount1, Radiance1, Variance2, SampleCount2, ConstantsBuffer, Size](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflectionsDenoise, Context);

				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "RTRefl_FFX_Prefilter";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/DenoiserReflection/Prefilter.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto CBSet = Context.GetDescriptorSet(Pipeline, 0);
				auto Set = Context.GetDescriptorSet(Pipeline, 1);
				Context.Device->UpdateDescriptorSet(CBSet, 0, 0, ConstantsBuffer);

				Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.GetRenderGraphTexture(AverageRadiance).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 4, 0, Context.GetRenderGraphTexture(InputRadiance).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 5, 0, Context.GetRenderGraphTexture(Variance1).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 6, 0, Context.GetRenderGraphTexture(SampleCount1).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 7, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());
				Context.Device->UpdateDescriptorSet(Set, 8, 0, Context.GetRenderGraphTexture(Radiance1).get());
				Context.Device->UpdateDescriptorSet(Set, 9, 0, Context.GetRenderGraphTexture(Variance2).get());
				Context.Device->UpdateDescriptorSet(Set, 10, 0, Context.GetRenderGraphTexture(SampleCount2).get());

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &CBSet);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 1, 1, &Set);
				Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { Size, 1 });
			});
		}

		// Temporal resolve and history output.
		{
			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(AverageRadiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Radiance1, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(ReprojectedRadiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Variance2, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(SampleCount2, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Radiance2, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Variance1, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(SampleCount1, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("RTRefl FFX ResolveTemporal", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, AverageRadiance, Radiance1, ReprojectedRadiance, Variance2, SampleCount2, Radiance2, Variance1, SampleCount1, ConstantsBuffer, Size](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflectionsDenoise, Context);

				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "RTRefl_FFX_ResolveTemporal";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/DenoiserReflection/ResolveTemporal.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto CBSet = Context.GetDescriptorSet(Pipeline, 0);
				auto Set = Context.GetDescriptorSet(Pipeline, 1);
				Context.Device->UpdateDescriptorSet(CBSet, 0, 0, ConstantsBuffer);

				Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(AverageRadiance).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(Radiance1).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.GetRenderGraphTexture(ReprojectedRadiance).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 4, 0, Context.GetRenderGraphTexture(Variance2).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 5, 0, Context.GetRenderGraphTexture(SampleCount2).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 6, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());
				Context.Device->UpdateDescriptorSet(Set, 7, 0, Context.GetRenderGraphTexture(Radiance2).get());
				Context.Device->UpdateDescriptorSet(Set, 8, 0, Context.GetRenderGraphTexture(Variance1).get());
				Context.Device->UpdateDescriptorSet(Set, 9, 0, Context.GetRenderGraphTexture(SampleCount1).get());

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &CBSet);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 1, 1, &Set);
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
	void RayTracedReflectionsPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext)
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

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferAlbedo, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.ReadBuffer(Textures.RadianceCache.DataBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			RayTracingIrradianceVolumes::Prepared IrradianceVolumes = RayTracingIrradianceVolumes::Prepare(Graph, Dependencies, CVar_RayTracingIrradianceVolumes.GetValue(), VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTReflectionRadiance, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTReflectionRays, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTReflectionRayPdf, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass("RayTraceReflections", RenderGraphPassType::Compute, Parameters, Dependencies, [RTReflectionRadiance, RTReflectionRays, RTReflectionRayPdf, Textures, View, UseRadianceCache, IrradianceVolumes](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflections, Context);

				static RayTracingPipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					RayTracingPipelineDesc Desc;
					Desc.Name = "RayTracedReflectionsPass";
					Desc.MaxRecursionDepth = 1;
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedReflections/RayTraceReflections.csd");

					Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
				}

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 2);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.Scene->TLAS);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(RTReflectionRadiance).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(RTReflectionRays).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.GetRenderGraphTexture(RTReflectionRayPdf).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(Textures.GBufferAlbedo).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 6, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 7, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				//Context.Device->UpdateDescriptorSet(DescriptorSet, 9, 0, Context.GetRenderGraphBuffer(Textures.RadianceCache.DataBuffer).get());
				RayTracingIrradianceVolumes::Bind(Context, DescriptorSet, IrradianceVolumes, Context.GetRenderGraphBuffer(Textures.RadianceCache.DataBuffer).get());

				RayTracedReflectionPassParameters Params{
					.CameraPosition = Vector4(View.CameraCur.Pos, 1),
					.MaxRoughness = CVar_MaxRoughness.GetValue(),
					.Random = (u32)GFrameNumber,
					.UseRadianceCache = (u32)UseRadianceCache,
				};

				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindGPUScene(Pipeline, false);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Params), &Params);
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

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferAlbedo, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(RTReflectionRadiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(RTReflectionRays, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(RTReflectionRayPdf, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(RTResolveResult, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(RTResolveHitDistance, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("RTReflResolve", RenderGraphPassType::Compute, Parameters, Dependencies, [RTReflectionRadiance, RTReflectionRays, RTReflectionRayPdf, RTResolveResult, RTResolveHitDistance, Textures, TraceSize](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflectionsDenoise, Context);

				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "ReflectionsResolve";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedReflections/ReflectionsResolve.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(RTReflectionRadiance).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(RTReflectionRays).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(RTReflectionRayPdf).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.GetRenderGraphTexture(Textures.GBufferAlbedo).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 6, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 7, 0, Context.GetRenderGraphTexture(RTResolveResult).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 8, 0, Context.GetRenderGraphTexture(RTResolveHitDistance).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 9, 0, Context.Scene->SceneBuffer);

				RayTracedReflectionsResolveParameters Params{
					.ImageSize = TraceSize,
					._Padding = iVector2(0, 0),
				};

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
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

			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(ReflectionsResult, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			if (ReflectionsHitDistance >= 0)
			{
				Dependencies.Read(ReflectionsHitDistance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			}
			Dependencies.Read(RTReflectionRays, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.Velocity, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(RTTemporalResult, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(RTTemporalSampleCount, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("RTReflTemporal", RenderGraphPassType::Compute, Parameters, Dependencies, [ReflectionsResult, ReflectionsHitDistance, RTReflectionRays, RTTemporalResult, RTTemporalSampleCount, ReflectionHistory, ReflectionSampleCountHistory, Textures, TraceSize, TemporalMaxSamples](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflectionsDenoise, Context);

				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "ReflectionsTemporal";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedReflections/ReflectionsTemporal.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(ReflectionsResult).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, ReflectionHistory, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(Textures.Velocity).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Textures.History.Depth, TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, ReflectionSampleCountHistory, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 6, 0, Context.GetRenderGraphTexture(RTReflectionRays).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 7, 0, Context.GetRenderGraphTexture(ReflectionsHitDistance).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 8, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 9, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 10, 0, Textures.History.Normals, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 11, 0, Textures.History.RoughnessMetallic, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 12, 0, Context.GetRenderGraphTexture(RTTemporalResult).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 13, 0, Context.GetRenderGraphTexture(RTTemporalSampleCount).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 14, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 15, 0, Context.Scene->SceneBuffer);

				RayTracedReflectionsTemporalParameters Params{
					.Size = TraceSize,
					.MaxSamples = TemporalMaxSamples,
					._Padding = 0,
				};

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { TraceSize, 1 });
			});

			Graph.ExtractTexture(RTTemporalResult, &Textures.History.RTReflectionsRadiance);
			Graph.ExtractTexture(RTTemporalSampleCount, &Textures.History.RTReflectionsSampleCount);
			ReflectionsResult = RTTemporalResult;
		}

		Textures.RTReflections = ReflectionsResult;
	}

}
