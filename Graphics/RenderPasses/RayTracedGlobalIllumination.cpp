#include "RenderPasses.h"

#include <imgui.h>

namespace Columbus
{

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedGI);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedGIDenoise);

	IMPLEMENT_GPU_PROFILING_COUNTER("RTGI", "RenderGraphGPU", GpuCounterRayTracedGI);
	IMPLEMENT_GPU_PROFILING_COUNTER("RTGI Denoise", "RenderGraphGPU", GpuCounterRayTracedGIDenoise);

	// https://github.com/GPUOpen-Effects/FidelityFX-SSSR/blob/master/sample/src/VK/Sources/SSSR.h
	struct ReflectionDenoiserConstants
	{
		Matrix invViewProjection;
		Matrix projection;
		Matrix invProjection;
		Matrix view;
		Matrix invView;
		Matrix prevViewProjection;
		unsigned int bufferDimensions[2];
		float inverseBufferDimensions[2];
		float temporalStabilityFactor;
		float depthBufferThickness;
		float roughnessThreshold;
		float varianceThreshold;
		uint32_t frameIndex;
		uint32_t maxTraversalIntersections;
		uint32_t minTraversalOccupancy;
		uint32_t mostDetailedMip;
		uint32_t samplesPerQuad;
		uint32_t temporalVarianceGuidedTracingEnabled;
	};

	static RenderGraphTextureRef Denoise(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, RenderGraphTextureRef RTGI_Tex)
	{
		RENDER_GRAPH_SCOPED_MARKER(Graph, "Reflections Denoise");

		BufferDesc CBDesc(sizeof(ReflectionDenoiserConstants), BufferType::Constant);

		// TODO: Global counter
		static u32 FrameNumber = 0;

		// TODO: manage constants and buffer uploads in a better way
		static Buffer* CB = Graph.Device->CreateBuffer(CBDesc, nullptr);
		static Buffer* UploadBuffers[MaxFramesInFlight]{ nullptr };

		const iVector2 Size = View.RenderSize;

		// update constants
		{
			if (UploadBuffers[0] == nullptr)
			{
				for (int i = 0; i < MaxFramesInFlight; i++)
				{
					BufferDesc UploadDesc(sizeof(ReflectionDenoiserConstants), BufferType::Constant);
					UploadDesc.HostVisible = true;

					UploadBuffers[i] = Graph.Device->CreateBuffer(UploadDesc, nullptr);
				}
			}

			ReflectionDenoiserConstants Constants;
			Constants.invViewProjection = View.CameraCur.GetViewProjection().GetInverted();
			Constants.projection = View.CameraCur.GetProjectionMatrix();
			Constants.invProjection = View.CameraCur.GetProjectionMatrix().GetInverted();
			Constants.view = View.CameraCur.GetViewMatrix();
			Constants.invView = View.CameraCur.GetViewMatrix().GetInverted();
			Constants.prevViewProjection = View.CameraPrev.GetViewProjection();
			Constants.bufferDimensions[0] = Size.X;
			Constants.bufferDimensions[1] = Size.Y;
			Constants.inverseBufferDimensions[0] = 1.0f / Size.X;
			Constants.inverseBufferDimensions[1] = 1.0f / Size.Y;
			Constants.temporalStabilityFactor = 0.7f;
			Constants.depthBufferThickness = 0.015f;
			Constants.roughnessThreshold = 1.0f;
			Constants.varianceThreshold = 0.0f;
			Constants.frameIndex = FrameNumber;
			Constants.maxTraversalIntersections = 128;
			Constants.minTraversalOccupancy = 4;
			Constants.mostDetailedMip = 0;
			Constants.samplesPerQuad = 1;
			Constants.temporalVarianceGuidedTracingEnabled = 1;

			u32 BufferId = FrameNumber % MaxFramesInFlight;
			void* Map = Graph.Device->MapBuffer(UploadBuffers[BufferId]);
			memcpy(Map, &Constants, sizeof(Constants));
			Graph.Device->UnmapBuffer(UploadBuffers[BufferId]);

			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Graph.AddPass("UploadConstantBuffer", RenderGraphPassType::Compute, Parameters, Dependencies, [BufferId](RenderGraphContext& Context)
			{
				Context.CommandBuffer->CopyBuffer(UploadBuffers[BufferId], CB, 0, 0, sizeof(ReflectionDenoiserConstants));
			});
		}

		RenderGraphTextureRef ReprojectedRadiance;
		RenderGraphTextureRef Radiance1, Radiance2;
		RenderGraphTextureRef AverageRadiance;
		RenderGraphTextureRef Variance1, Variance2;
		RenderGraphTextureRef SampleCount1, SampleCount2;

		// allocate textures
		{
			TextureDesc2 Desc;
			Desc.Width = (u32)Size.X;
			Desc.Height = (u32)Size.Y;
			Desc.Usage = TextureUsage::Storage | TextureUsage::Sampled;

			{
				Desc.Format = TextureFormat::RGBA16F;
				ReprojectedRadiance = Graph.CreateTexture(Desc, "Reflection Denoiser ReprojectedRadiance");
				Radiance1 = Graph.CreateTexture(Desc, "Reflection Denoiser Radiance 1");
				Radiance2 = Graph.CreateTexture(Desc, "Reflection Denoiser Radiance 2");
			}

			{
				Desc.Format = TextureFormat::R16F;
				Variance1 = Graph.CreateTexture(Desc, "Reflection Denoiser Variance 1");
				Variance2 = Graph.CreateTexture(Desc, "Reflection Denoiser Variance 2");
				SampleCount1 = Graph.CreateTexture(Desc, "Reflection Denoiser SampleCount 1");
				SampleCount2 = Graph.CreateTexture(Desc, "Reflection Denoiser SampleCount 2");

				Graph.CreateHistoryTexture(&Textures.History.RTGI_History.Variance, Desc, "Reflection Denoiser Variance History");
				Graph.CreateHistoryTexture(&Textures.History.RTGI_History.SampleCount, Desc, "Reflection Denoiser SampleCount History");
			}

			{
				Desc.Width = (u32)Size.X / 8;
				Desc.Height = (u32)Size.Y / 8;
				Desc.Format = TextureFormat::RGBA16F;
				AverageRadiance = Graph.CreateTexture(Desc, "Reflection Denoiser AverageRadiance");

				Graph.CreateHistoryTexture(&Textures.History.RTGI_History.AverageRadiance, Desc, "Reflection Denoiser AverageRadiance History");
			}
		}

		struct
		{
			RenderGraphTextureRef Radiance1, Radiance2;
			RenderGraphTextureRef Variance1, Variance2;
			RenderGraphTextureRef SampleCount1, SampleCount2;
		} CaptureParameters;
		CaptureParameters.Radiance1 = Radiance1;
		CaptureParameters.Radiance2 = Radiance2;
		CaptureParameters.Variance1 = Variance1;
		CaptureParameters.Variance2 = Variance2;
		CaptureParameters.SampleCount1 = SampleCount1;
		CaptureParameters.SampleCount2 = SampleCount2;

		// reproject
		{

			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			// TODO: register external textures
			//Dependencies.Read(Textures.History.Depth, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			// roughness history
			// normal history

			Dependencies.Read(RTGI_Tex, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			// radiance history
			Dependencies.Read(Textures.Velocity, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			// avg radiance history
			// variance history
			// sample count history

			Dependencies.Write(ReprojectedRadiance, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(AverageRadiance, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Variance1, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(SampleCount1, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("Reproject", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, RTGI_Tex, ReprojectedRadiance, AverageRadiance, Variance1, SampleCount1, Size](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIDenoise, Context);

				// TODO: shader system
				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "ReflectionDenoise_Reproject";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/DenoiserReflection/Reproject.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto CBSet = Context.GetDescriptorSet(Pipeline, 0);
				auto Set   = Context.GetDescriptorSet(Pipeline, 1);

				Context.Device->UpdateDescriptorSet(CBSet, 0, 0, CB);

				Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 3, 0, Textures.History.Depth, TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);


				Context.Device->UpdateDescriptorSet(Set, 4, 0, Textures.History.RoughnessMetallic, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 5, 0, Textures.History.Normals, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

				Context.Device->UpdateDescriptorSet(Set, 6, 0, Context.GetRenderGraphTexture(RTGI_Tex).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 7, 0, Textures.History.RTGI_History.Radiance, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 8, 0, Context.GetRenderGraphTexture(Textures.Velocity).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

				Context.Device->UpdateDescriptorSet(Set, 9, 0, Textures.History.RTGI_History.AverageRadiance, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 10, 0, Textures.History.RTGI_History.Variance, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 11, 0, Textures.History.RTGI_History.SampleCount, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

				Context.Device->UpdateDescriptorSet(Set, 12, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());

				Context.Device->UpdateDescriptorSet(Set, 13, 0, Context.GetRenderGraphTexture(ReprojectedRadiance).get());
				Context.Device->UpdateDescriptorSet(Set, 14, 0, Context.GetRenderGraphTexture(AverageRadiance).get());
				Context.Device->UpdateDescriptorSet(Set, 15, 0, Context.GetRenderGraphTexture(Variance1).get());
				Context.Device->UpdateDescriptorSet(Set, 16, 0, Context.GetRenderGraphTexture(SampleCount1).get());

				const int GroupSize = 8; // 8x8
				const iVector2 Groups = (Size + GroupSize - 1) / GroupSize;

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &CBSet);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 1, 1, &Set);
				Context.CommandBuffer->Dispatch((u32)Groups.X, (u32)Groups.Y, 1);
			});
		}

		// prefilter (spational denoise)
		{
			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Dependencies.Read(AverageRadiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(RTGI_Tex, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Variance1, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(SampleCount1, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Dependencies.Write(Radiance1, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Variance2, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(SampleCount2, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("Prefilter", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, RTGI_Tex, CaptureParameters, AverageRadiance, Size](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIDenoise, Context);

				// TODO: shader system
				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "ReflectionDenoise_Prefilter";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/DenoiserReflection/Prefilter.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto CBSet = Context.GetDescriptorSet(Pipeline, 0);
				auto Set   = Context.GetDescriptorSet(Pipeline, 1);

				Context.Device->UpdateDescriptorSet(CBSet, 0, 0, CB);

				Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

				Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.GetRenderGraphTexture(AverageRadiance).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 4, 0, Context.GetRenderGraphTexture(RTGI_Tex).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 5, 0, Context.GetRenderGraphTexture(CaptureParameters.Variance1).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 6, 0, Context.GetRenderGraphTexture(CaptureParameters.SampleCount1).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

				Context.Device->UpdateDescriptorSet(Set, 7, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());

				Context.Device->UpdateDescriptorSet(Set, 8, 0, Context.GetRenderGraphTexture(CaptureParameters.Radiance1).get());
				Context.Device->UpdateDescriptorSet(Set, 9, 0, Context.GetRenderGraphTexture(CaptureParameters.Variance2).get());
				Context.Device->UpdateDescriptorSet(Set, 10, 0, Context.GetRenderGraphTexture(CaptureParameters.SampleCount2).get());

				const int GroupSize = 8; // 8x8
				const iVector2 Groups = (Size + GroupSize - 1) / GroupSize;

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &CBSet);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 1, 1, &Set);
				Context.CommandBuffer->Dispatch((u32)Groups.X, (u32)Groups.Y, 1);
			});
		}

		// resolve (temporal denoise)
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

			Graph.AddPass("Resolve Temporal", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, RTGI_Tex, CaptureParameters, ReprojectedRadiance, AverageRadiance, Size](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIDenoise, Context);

				// TODO: shader system
				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "ReflectionDenoise_ResolveTemporal";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/DenoiserReflection/ResolveTemporal.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto CBSet = Context.GetDescriptorSet(Pipeline, 0);
				auto Set = Context.GetDescriptorSet(Pipeline, 1);

				Context.Device->UpdateDescriptorSet(CBSet, 0, 0, CB);

				Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(AverageRadiance).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(CaptureParameters.Radiance1).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE); // in_radiance
				Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.GetRenderGraphTexture(ReprojectedRadiance).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 4, 0, Context.GetRenderGraphTexture(CaptureParameters.Variance2).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 5, 0, Context.GetRenderGraphTexture(CaptureParameters.SampleCount2).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

				Context.Device->UpdateDescriptorSet(Set, 6, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());

				Context.Device->UpdateDescriptorSet(Set, 7, 0, Context.GetRenderGraphTexture(CaptureParameters.Radiance2).get());
				Context.Device->UpdateDescriptorSet(Set, 8, 0, Context.GetRenderGraphTexture(CaptureParameters.Variance1).get());
				Context.Device->UpdateDescriptorSet(Set, 9, 0, Context.GetRenderGraphTexture(CaptureParameters.SampleCount1).get());

				const int GroupSize = 8; // 8x8
				const iVector2 Groups = (Size + GroupSize - 1) / GroupSize;

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &CBSet);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 1, 1, &Set);
				Context.CommandBuffer->Dispatch((u32)Groups.X, (u32)Groups.Y, 1);
			});
		}

		Graph.ExtractTexture(AverageRadiance, &Textures.History.RTGI_History.AverageRadiance);
		Graph.ExtractTexture(Variance1, &Textures.History.RTGI_History.Variance);
		Graph.ExtractTexture(SampleCount1, &Textures.History.RTGI_History.SampleCount);

		// TODO: Global counter
		FrameNumber++;

		return Radiance2;
	}

	void RTGIDenoiserHistory::Destroy(SPtr<DeviceVulkan> Device)
	{
		Device->DestroyTexture(Radiance);
		Device->DestroyTexture(SampleCount);
	}

	struct SimpleDenoiseSpatialParameters
	{
		iVector2 Size;
		int StepSize;
		int MaxSamples;
		int DominationNumber;
	};

	struct SimpleDenoiserTemporalParameters
	{
		Matrix ProjectionInv;
		//Matrix ReprojectionMatrix;
		Matrix ViewProjectionInv;
		Matrix PrevViewProjection;
		iVector2 Size;
		int MaxSamples;
	};

	struct DownsampleGBufferParameters
	{
		iVector2 Size;
		int DownsampleFactor;
	};

	struct UpsampleRTGIParameters
	{
		iVector2 Size;
		int DownsampleFactor;
	};

	struct DownsampledTextures
	{
		RenderGraphTextureRef Depth;
		RenderGraphTextureRef Normal;
		RenderGraphTextureRef WorldPos;
		RenderGraphTextureRef DepthHistory;
		RenderGraphTextureRef Velocity;
	};

	static DownsampledTextures DownsampleGBuffer(RenderGraph& Graph, SceneTextures& Textures, int DownsampleFactor)
	{
		const iVector2 Size = Graph.GetTextureSize2D(Textures.GBufferNormal) / DownsampleFactor;

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
		TextureDesc2 WorldPosDesc  = CommonDesc;
		TextureDesc2 DepthHistDesc = CommonDesc;
		TextureDesc2 VelocityDesc  = CommonDesc;

		DepthDesc.Format     = TextureFormat::R16;
		NormalDesc.Format    = Graph.GetTextureDesc(Textures.GBufferNormal).Format;
		WorldPosDesc.Format  = Graph.GetTextureDesc(Textures.GBufferWP).Format;
		DepthHistDesc.Format = TextureFormat::R16;
		VelocityDesc.Format  = Graph.GetTextureDesc(Textures.Velocity).Format;

		Result.Depth        = Graph.CreateTexture(DepthDesc,    "Downsampled Depth");
		Result.Normal       = Graph.CreateTexture(NormalDesc,   "Downsampled Normals");
		Result.WorldPos     = Graph.CreateTexture(WorldPosDesc, "Downsampled WorldPos");
		Result.DepthHistory = Graph.CreateTexture(DepthHistDesc, "Downsampled DepthHistory");
		Result.Velocity     = Graph.CreateTexture(VelocityDesc, "Downsampled Velocity");

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(Textures.Velocity, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Dependencies.Write(Result.Depth, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(Result.Normal, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(Result.WorldPos, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(Result.DepthHistory, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(Result.Velocity, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("Downsample GBuffer", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, Result, Size, DownsampleFactor](RenderGraphContext& Context)
		{
			static ComputePipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				ComputePipelineDesc Desc;
				Desc.Name = "RTGI_Downsample_GBuffer";
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedGlobalIllumination/Downsample.csd");
				Pipeline = Context.Device->CreateComputePipeline(Desc);
			}

			auto Set = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 3, 0, Textures.History.Depth, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 4, 0, Context.GetRenderGraphTexture(Textures.Velocity).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

			Context.Device->UpdateDescriptorSet(Set, 5, 0, Context.GetRenderGraphTexture(Result.Depth).get());
			Context.Device->UpdateDescriptorSet(Set, 6, 0, Context.GetRenderGraphTexture(Result.Normal).get());
			Context.Device->UpdateDescriptorSet(Set, 7, 0, Context.GetRenderGraphTexture(Result.WorldPos).get());
			Context.Device->UpdateDescriptorSet(Set, 8, 0, Context.GetRenderGraphTexture(Result.DepthHistory).get());
			Context.Device->UpdateDescriptorSet(Set, 9, 0, Context.GetRenderGraphTexture(Result.Velocity).get());

			DownsampleGBufferParameters Params{
				.Size = Size,
				.DownsampleFactor = DownsampleFactor,
			};

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
			Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
			Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { Size, 1 });
		});

		return Result;
	}

	template <int Perm>
	static void SimpleDenoiseSpatial(RenderGraph& Graph, SceneTextures& Textures, DownsampledTextures DownTextures, iVector2 Size, int StepSize, RenderGraphTextureRef Radiance1, RenderGraphTextureRef Radiance2, int MaxSamples, int DominationSampleNumber)
	{
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(DownTextures.Depth, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(DownTextures.Normal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(Radiance1, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(Radiance2, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("Spatial", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, DownTextures, Radiance1, Radiance2, Size, StepSize, MaxSamples, DominationSampleNumber](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIDenoise, Context);

			// TODO: shader system
			static ComputePipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				ComputePipelineDesc Desc;
				Desc.Name = "RTGI_Denoiser_Spatial";
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedGlobalIllumination/Spatial.csd");
				Pipeline = Context.Device->CreateComputePipeline(Desc);
			}

			auto Set = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(DownTextures.Depth).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(DownTextures.Normal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 2, 0, Textures.History.RTGI_History.SampleCount, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.GetRenderGraphTexture(Radiance1).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

			Context.Device->UpdateDescriptorSet(Set, 4, 0, Context.GetRenderGraphTexture(Radiance2).get());

			SimpleDenoiseSpatialParameters Params{
				.Size = Size,
				.StepSize = StepSize,
				.MaxSamples = MaxSamples,
				.DominationNumber = DominationSampleNumber,
			};

			const int GroupSize = 8; // 8x8
			const iVector2 Groups = (Size + GroupSize - 1) / GroupSize;

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
			Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
			Context.CommandBuffer->Dispatch((u32)Groups.X, (u32)Groups.Y, 1);
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
			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(DownTextures.Velocity, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Radiance1, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			//Dependencies.Read(Textures.History.RTGI_History.Radiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(Radiance2, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(DownTextures.Depth, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(DownTextures.DepthHistory, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Matrix ViewProjection = View.CameraCur.GetViewProjection();
			Matrix InvViewProjection = View.CameraCur.GetViewProjection().GetInverted();
			Matrix ProjectionInv = View.CameraCur.GetProjectionMatrix().GetInverted();
			Matrix PrevViewProjection = View.CameraPrev.GetViewProjection();
			Matrix Reprojection = InvViewProjection * PrevViewProjection;

			Graph.AddPass("Temporal", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, DownTextures, Radiance1, Radiance2, Size, ProjectionInv, InvViewProjection, Reprojection, PrevViewProjection](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGIDenoise, Context);

				// TODO: shader system
				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "RTGI_Denoiser_Temporal";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedGlobalIllumination/Temporal.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto Set = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(DownTextures.Velocity).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(Radiance1).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 2, 0, Textures.History.RTGI_History.Radiance, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.GetRenderGraphTexture(DownTextures.Depth).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 4, 0, Context.GetRenderGraphTexture(DownTextures.DepthHistory).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 5, 0, Textures.History.RTGI_History.SampleCount);

				Context.Device->UpdateDescriptorSet(Set, 6, 0, Context.GetRenderGraphTexture(Radiance2).get());
				Context.Device->UpdateDescriptorSet(Set, 7, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());

				SimpleDenoiserTemporalParameters Params{
					.ProjectionInv = ProjectionInv,
					.ViewProjectionInv = InvViewProjection,
					.PrevViewProjection = PrevViewProjection,
					//.ReprojectionMatrix = Reprojection,
					.Size = Size,
					.MaxSamples = MaxSamples,
				};

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
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

		TextureDesc2 Desc;
		Desc.Width = (u32)Size.X;
		Desc.Height = (u32)Size.Y;
		Desc.Usage = TextureUsage::Sampled | TextureUsage::Storage;
		Desc.Format = TextureFormat::RGBA16F;

		RenderGraphTextureRef Result = Graph.CreateTexture(Desc, "RTGI_Upsampled");

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(RTGI_Tex, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(DownTextures.Depth, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(Result, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("Upsample", RenderGraphPassType::Compute, Parameters, Dependencies, [RTGI_Tex, Size, Result, DownsampleFactor, Textures, DownTextures](RenderGraphContext& Context)
		{
			// TODO: shader system
			static ComputePipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				ComputePipelineDesc Desc;
				Desc.Name = "RTGI_Upsample";
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedGlobalIllumination/Upsample.csd");
				Pipeline = Context.Device->CreateComputePipeline(Desc);
			}

			auto Set = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(RTGI_Tex).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.GetRenderGraphTexture(DownTextures.Depth).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 4, 0, Context.GetRenderGraphTexture(Result).get());

			UpsampleRTGIParameters Params{
				.Size = Size,
				.DownsampleFactor = DownsampleFactor,
			};

			const int GroupSize = 8; // 8x8
			const iVector2 Groups = (Size + GroupSize - 1) / GroupSize;

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
			Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
			Context.CommandBuffer->Dispatch((u32)Groups.X, (u32)Groups.Y, 1);
		});

		return Result;
	}

	struct RTGI_Parameters
	{
		Vector3 CameraPosition;
		u32   Random;
		float DiffuseBoost;
		u32   UseRadianceCache;
		float UpscaleFactor;
	};

	// diffuse GI, one sample
	void RayTracedGlobalIlluminationPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext)
	{
		static bool UseDenoiser = true;
		static bool UseRadianceCache = false;
		static float DiffuseBoost = 1.0f;
		static int DownsampleFactor = 2;

		// debug ui
		if (ImGui::GetCurrentContext())
		{
			if (ImGui::Begin("RTGI"))
			{
				ImGui::Checkbox("Denoise", &UseDenoiser);
				ImGui::Checkbox("Radiance Cache", &UseRadianceCache);
				ImGui::SliderFloat("Diffuse Boost", &DiffuseBoost, 1.0f, 5.0f);
				ImGui::SliderInt("Downsample Factor", &DownsampleFactor, 1, 4);
			}
			ImGui::End();
		}

		DownsampledTextures DownsampledGBuffer = DownsampleGBuffer(Graph, Textures, DownsampleFactor);

		const iVector2 GIResolution = View.RenderSize / DownsampleFactor;

		TextureDesc2 Desc{
			.Usage = TextureUsage::Storage | TextureUsage::Sampled,
			.Width = (u32)GIResolution.X,
			.Height = (u32)GIResolution.Y,
			.Format = TextureFormat::RGBA16F,
		};

		RenderGraphTextureRef RTGI_Tex = Graph.CreateTexture(Desc, "RayTracedGI");
		Graph.CreateHistoryTexture(&Textures.History.RTGI_History.Radiance, Desc, "RayTracedGI History");

		RenderPassParameters Parameters;

		// TODO: Get rid of WP

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(DownsampledGBuffer.WorldPos, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		Dependencies.Read(DownsampledGBuffer.Depth, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		Dependencies.Read(DownsampledGBuffer.Normal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		Dependencies.ReadBuffer(Textures.RadianceCache.DataBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		Dependencies.Write(RTGI_Tex, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

		Graph.AddPass("RayTraceGI", RenderGraphPassType::Compute, Parameters, Dependencies, [RTGI_Tex, Textures, View, GIResolution, DownsampledGBuffer](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedGI, Context);

			static RayTracingPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				RayTracingPipelineDesc Desc;
				Desc.Name = "RayTracedGI";
				Desc.MaxRecursionDepth = 1;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedGlobalIllumination/RayTracedGI.csd");

				Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
			}

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 2);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.Scene->TLAS);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(RTGI_Tex).get());
			Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(DownsampledGBuffer.Normal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.GetRenderGraphTexture(DownsampledGBuffer.WorldPos).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(DownsampledGBuffer.Depth).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			//Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, Context.GetRenderGraphBuffer(Textures.RadianceCache.DataBuffer).get());

			RTGI_Parameters Params{
				.CameraPosition = View.CameraCur.Pos,
				.Random = (u32)rand() % 2000,
				.DiffuseBoost = DiffuseBoost,
				.UseRadianceCache = (u32)UseRadianceCache,
				.UpscaleFactor = (float)DownsampleFactor,
			};

			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.BindGPUScene(Pipeline, false);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &DescriptorSet);
			Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Params), &Params);
			Context.CommandBuffer->TraceRays(Pipeline, GIResolution.X, GIResolution.Y, 1);
		});

		if (UseDenoiser)
		{
			//RTGI_Tex = Denoise(Graph, View, Textures, RTGI_Tex);
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
