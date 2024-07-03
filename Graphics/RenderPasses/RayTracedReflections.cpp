#include "RenderPasses.h"

#include <Lib/imgui/imgui.h>

namespace Columbus
{

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedReflections);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterRayTracedReflectionsDenoise);

	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Reflections", "RenderGraphGPU", GpuCounterRayTracedReflections);
	IMPLEMENT_GPU_PROFILING_COUNTER("Raytraced Reflections Denoise", "RenderGraphGPU", GpuCounterRayTracedReflectionsDenoise);

	struct RayTracedReflectionPassParameters
	{
		Vector4 CameraPosition;
		u32 Random;
		u32 UseRadianceCache;
	};

	struct RayTracedReflectionsDenoiseParamters
	{
		iVector2 ImageSize;
	};

	struct RayTracedReflectionsResolveParameters
	{
		iVector2 ImageSize;
	};

	// TODO: downscale reflections resolution and then upscale (FSR1?)
	// TODO: roughness cut for RT reflections (0.4-0.5)
	// TODO: denoise
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

		static bool Blur = false;
		static bool Resolve = false;
		static bool UseRadianceCache = false;

		if (ImGui::GetCurrentContext())
		{
			if (ImGui::Begin("RT Reflections Debug"))
			{
				ImGui::Checkbox("Blur", &Blur);
				ImGui::Checkbox("Resolve", &Resolve);
				ImGui::Checkbox("Radiance cache", &UseRadianceCache);
			}
			ImGui::End();
		}

		RenderGraphTextureRef RTReflectionRadiance = Graph.CreateTexture(Desc, "RTReflRadiance");
		RenderGraphTextureRef RTReflectionRays = Graph.CreateTexture(Desc, "RTReflRays");

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

			Graph.AddPass("RayTraceReflections", RenderGraphPassType::Compute, Parameters, Dependencies, [RTReflectionRadiance, RTReflectionRays, Textures, View](RenderGraphContext& Context)
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
				Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.GetRenderGraphTexture(Textures.GBufferAlbedo).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 6, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 7, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 8, 0, Context.GetRenderGraphBuffer(Textures.RadianceCache.DataBuffer).get());

				RayTracedReflectionPassParameters Params{
					.CameraPosition = Vector4(View.CameraCur.Pos, 1),
					.Random = (u32)rand() % 2000,
					.UseRadianceCache = (u32)UseRadianceCache,
				};

				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindGPUScene(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Params), &Params);
				Context.CommandBuffer->TraceRays(Pipeline, View.RenderSize.X, View.RenderSize.Y, 1);
			});
		}

		if (Blur)
		{
			RenderGraphTextureRef RTReflectionsDenoised = Graph.CreateTexture(Desc, "RayTracedReflectionDenoised");

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(RTReflectionRadiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTReflectionsDenoised, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass("Blur", RenderGraphPassType::Compute, Parameters, Dependencies, [RTReflectionRadiance, RTReflectionsDenoised, View](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRayTracedReflectionsDenoise, Context);

				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "RayTracedReflectionsBlur";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedReflections/SimpleDenoise.csd");

					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(RTReflectionRadiance).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(RTReflectionsDenoised).get());

				RayTracedReflectionsDenoiseParamters Params{
					.ImageSize = View.RenderSize
				};

				iVector2 GroupsSize = (View.RenderSize + 7) / 8; // 8x8 group

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.CommandBuffer->Dispatch(GroupsSize.X, GroupsSize.Y, 1);
			});

			RTReflectionRadiance = RTReflectionsDenoised;
		}

		Textures.RTReflections = RTReflectionRadiance;

		// reflections resolve
		if (Resolve)
		{
			RenderGraphTextureRef RTResolveResult = Graph.CreateTexture(Desc, "RTReflResolved");

			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Textures.GBufferAlbedo, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferRM, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferDS, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(RTReflectionRadiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(RTReflectionRays, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTResolveResult, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass("RTReflResolve", RenderGraphPassType::Compute, Parameters, Dependencies, [RTReflectionRadiance, RTReflectionRays, RTResolveResult, Textures, TraceSize](RenderGraphContext& Context)
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
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(RTReflectionRadiance).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(RTReflectionRays).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferAlbedo).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 4, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 5, 0, Context.GetRenderGraphTexture(Textures.GBufferRM).get());
				Context.Device->UpdateDescriptorSet(DescriptorSet, 6, 0, Context.GetRenderGraphTexture(Textures.GBufferDS).get(), TextureBindingFlags::AspectDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 7, 0, Context.GetRenderGraphTexture(RTResolveResult).get());

				RayTracedReflectionsResolveParameters Params{
					.ImageSize = TraceSize
				};

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { TraceSize, 1 });
			});

			Textures.RTReflections = RTResolveResult;
		}
	}

}
