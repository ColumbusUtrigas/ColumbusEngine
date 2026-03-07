#include "RenderPasses.h"
#include "Core/CVar.h"

#include <Lib/imgui/imgui.h>

namespace Columbus
{

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedReflections);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedReflectionsDenoise);

	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Reflections", "RenderGraphGPU", GpuCounterRayTracedReflections);
	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Reflections Denoise", "RenderGraphGPU", GpuCounterRayTracedReflectionsDenoise);

	ConsoleVariable<float> CVar_MaxRoughness("r.RTReflection.MaxRoughness", "Max roughness to trace, default - 0.5", 0.5f);

	struct RayTracedReflectionPassParameters
	{
		Vector4 CameraPosition;
		float MaxRoughness;
		u32 Random;
		u32 UseRadianceCache;
	};

	struct RayTracedReflectionsResolveParameters
	{
		Vector4 CameraPosition;
		iVector2 ImageSize;
		iVector2 _Padding;
	};

	struct RayTracedReflectionsTemporalParameters
	{
		Matrix ProjectionInv;
		Matrix ViewProjectionInv;
		Matrix PrevViewProjection;
		iVector2 Size;
		int MaxSamples;
		int _Padding;
	};

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

		static bool Resolve = true;
		static bool Temporal = true;
		static bool UseRadianceCache = false;
		static int TemporalMaxSamples = 16;

		if (ImGui::GetCurrentContext())
		{
			if (ImGui::Begin("RT Reflections Debug"))
			{
				ImGui::Checkbox("Resolve", &Resolve);
				ImGui::Checkbox("Temporal", &Temporal);
				ImGui::SliderInt("Temporal Max Samples", &TemporalMaxSamples, 1, 64);
				ImGui::Checkbox("Radiance cache", &UseRadianceCache);
			}
			ImGui::End();
		}

		RenderGraphTextureRef RTReflectionRadiance = Graph.CreateTexture(Desc, "RTReflRadiance");
		RenderGraphTextureRef RTReflectionRays = Graph.CreateTexture(Desc, "RTReflRays");
		RenderGraphTextureRef RTReflectionRayPdf = Graph.CreateTexture(SingleChannelDesc, "RTReflRayPdf");

		{
			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferAlbedo, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.ReadBuffer(Textures.RadianceCache.DataBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTReflectionRadiance, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTReflectionRays, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTReflectionRayPdf, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass("RayTraceReflections", RenderGraphPassType::Compute, Parameters, Dependencies, [RTReflectionRadiance, RTReflectionRays, RTReflectionRayPdf, Textures, View](RenderGraphContext& Context)
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
				Context.Device->UpdateDescriptorSet(DescriptorSet, 6, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 7, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 8, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				//Context.Device->UpdateDescriptorSet(DescriptorSet, 9, 0, Context.GetRenderGraphBuffer(Textures.RadianceCache.DataBuffer).get());

				RayTracedReflectionPassParameters Params{
					.CameraPosition = Vector4(View.CameraCur.Pos, 1),
					.MaxRoughness = CVar_MaxRoughness.GetValue(),
					.Random = (u32)rand() % 2000,
					.UseRadianceCache = (u32)UseRadianceCache,
				};

				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindGPUScene(Pipeline, false);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Params), &Params);
				Context.CommandBuffer->TraceRays(Pipeline, View.RenderSize.X, View.RenderSize.Y, 1);
			});
		}

		RenderGraphTextureRef ReflectionsResult = RTReflectionRadiance;

		// BRDF/PDF-based spatial reuse pass
		if (Resolve)
		{
			RenderGraphTextureRef RTResolveResult = Graph.CreateTexture(Desc, "RTReflResolved");

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferAlbedo, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(RTReflectionRadiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(RTReflectionRays, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(RTReflectionRayPdf, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(RTResolveResult, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass("RTReflResolve", RenderGraphPassType::Compute, Parameters, Dependencies, [RTReflectionRadiance, RTReflectionRays, RTReflectionRayPdf, RTResolveResult, Textures, TraceSize, View](RenderGraphContext& Context)
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
				Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 6, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 7, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 8, 0, Context.GetRenderGraphTexture(RTResolveResult).get());

				RayTracedReflectionsResolveParameters Params{
					.CameraPosition = Vector4(View.CameraCur.Pos, 1),
					.ImageSize = TraceSize,
					._Padding = iVector2(0, 0),
				};

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { TraceSize, 1 });
			});

			ReflectionsResult = RTResolveResult;
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
			Dependencies.Read(Textures.Velocity, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(RTTemporalResult, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.Write(RTTemporalSampleCount, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Matrix InvViewProjection = View.CameraCur.GetViewProjection().GetInverted();
			Matrix ProjectionInv = View.CameraCur.GetProjectionMatrix().GetInverted();
			Matrix PrevViewProjection = View.CameraPrev.GetViewProjection();

			Graph.AddPass("RTReflTemporal", RenderGraphPassType::Compute, Parameters, Dependencies, [ReflectionsResult, RTTemporalResult, RTTemporalSampleCount, ReflectionHistory, ReflectionSampleCountHistory, Textures, TraceSize, ProjectionInv, InvViewProjection, PrevViewProjection](RenderGraphContext& Context)
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
				Context.Device->UpdateDescriptorSet(DescriptorSet, 6, 0, Context.GetRenderGraphTexture(RTTemporalResult).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 7, 0, Context.GetRenderGraphTexture(RTTemporalSampleCount).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 8, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());

				RayTracedReflectionsTemporalParameters Params{
					.ProjectionInv = ProjectionInv,
					.ViewProjectionInv = InvViewProjection,
					.PrevViewProjection = PrevViewProjection,
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
