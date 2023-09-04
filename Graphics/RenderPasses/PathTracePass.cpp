#include "Core/Core.h"
#include "Graphics/Core/Pipelines.h"
#include "Graphics/Core/Texture.h"
#include "Graphics/GPUScene.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	std::string PathTraceDisplayVertexShader = R"(#version 460 core
	layout (location = 0) out vec2 texcoords;

	const vec3 verts[3] = vec3[](
		vec3(-3, -1, 0),
		vec3(1, 3, 0),
		vec3(1, -1, 0)
	);

	void main() {
		gl_Position = vec4(verts[gl_VertexIndex], 1);
		texcoords = 0.5 * gl_Position.xy + vec2(0.5);
	}
	)";

	std::string PathTraceDisplayPixelShader = R"(#version 460 core
		layout(location = 0) in vec2 texcoords;

		layout(binding = 0, set = 0, rgba32f) uniform image2D img;

		layout(location = 0) out vec4 RT0;

		layout(push_constant) uniform params
		{
			uint frameNumber;
		} Parameters;

		void main() {
			vec3 color = imageLoad(img, ivec2(texcoords * vec2(1280,720))).rgb / float(Parameters.frameNumber);
			RT0 = vec4(color, 1.0);
		}
	)";

	// no need to GPUParametrize push constants?
	struct PathTraceParameters
	{
		GPUCamera MainCamera;
		int frameNumber;
		int reset;
		int bounces;
	};

	struct PathTraceDisplayParameters
	{
		uint FrameNumber;
	};

	void RenderPathTraced(RenderGraph& Graph, const Camera& MainCamera, const iVector2& WindowSize)
	{
		TextureDesc2 PTTextureDesc {
			.Usage = TextureUsage::Storage,
			.Width = (uint32)WindowSize.X, .Height = (uint32)WindowSize.Y,
			.Format = TextureFormat::RGBA16F,
		};
		RenderGraphTextureRef RTImage = Graph.CreateTexture(PTTextureDesc, "PathTraceTexture");

		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies;
		Dependencies.Write(RTImage, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

		Graph.AddPass("PathTrace", RenderGraphPassType::Compute, Parameters, Dependencies, [RTImage, &MainCamera](RenderGraphContext& Context)
		{
			static RayTracingPipeline* PTPipeline = nullptr;
			if (PTPipeline == nullptr)
			{
				auto ShaderSource = LoadShaderFile("PathTrace.glsl");

				RayTracingPipelineDesc Desc{};
				Desc.Name = "RTXON";
				Desc.RayGen = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Raygen, ShaderLanguage::GLSL);
				Desc.Miss = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Miss, ShaderLanguage::GLSL);
				Desc.ClosestHit = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::ClosestHit, ShaderLanguage::GLSL);
				Desc.MaxRecursionDepth = 1;
				PTPipeline = Context.Device->CreateRayTracingPipeline(Desc);
			}

			auto RTSet = Context.GetDescriptorSet(PTPipeline, 7);
			Context.Device->UpdateDescriptorSet(RTSet, 0, 0, Context.Scene->TLAS); // TODO: move to unified scene set
			Context.Device->UpdateDescriptorSet(RTSet, 1, 0, RTImage.get());

			GPUCamera UpdatedCamera = GPUCamera(MainCamera);
			Context.Scene->Dirty = Context.Scene->MainCamera != UpdatedCamera; // TODO: move to the main rendering system
			Context.Scene->MainCamera = UpdatedCamera;

			bool reset = Context.Scene->Dirty;
			int frame = rand() % 2000;
			int bounces = 2;

			PathTraceParameters rayParams = { UpdatedCamera, (int)frame, (int)reset, bounces };

			Context.CommandBuffer->BindRayTracingPipeline(PTPipeline);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(PTPipeline, 7, 1, &RTSet);
			Context.BindGPUScene(PTPipeline);

			Context.CommandBuffer->PushConstantsRayTracing(PTPipeline, ShaderType::Raygen, 0, sizeof(rayParams), &rayParams);
			Context.CommandBuffer->TraceRays(PTPipeline, 1280, 720, 1);
		});
	}

	/*void PathTracePass::Setup(RenderGraphContext& Context)
	{
		auto ShaderSource = LoadShaderFile("PathTrace.glsl");

		RayTracingPipelineDesc Desc{};
		Desc.Name = "RTXON";
		Desc.RayGen = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Raygen, ShaderLanguage::GLSL);
		Desc.Miss = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Miss, ShaderLanguage::GLSL);
		Desc.ClosestHit = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::ClosestHit, ShaderLanguage::GLSL);
		Desc.MaxRecursionDepth = 1;
		PTPipeline = Context.Device->CreateRayTracingPipeline(Desc);
	}

	RenderPass::TExecutionFunc PathTracePass::Execute2(RenderGraphContext& Context)
	{
		return [this](RenderGraphContext& Context)
		{
			TextureDesc2 RTDesc;
			RTDesc.Usage = TextureUsage::Storage;
			RTDesc.Width = 1280; // TODO: swapchain and RT resize
			RTDesc.Height = 720;
			RTDesc.Format = TextureFormat::RGBA16F;
			auto RTImage = Context.GetRenderTarget(RenderTargetName, RTDesc);

			auto RTSet = Context.GetDescriptorSet(PTPipeline, 7);
			Context.Device->UpdateDescriptorSet(RTSet, 0, 0, Context.Scene->TLAS); // TODO: move to unified scene set
			Context.Device->UpdateDescriptorSet(RTSet, 1, 0, RTImage);

			GPUCamera UpdatedCamera = GPUCamera(MainCamera);
			Context.Scene->Dirty = Context.Scene->MainCamera != UpdatedCamera; // TODO: move to the main rendering system
			Context.Scene->MainCamera = UpdatedCamera;

			bool reset = Context.Scene->Dirty;
			int frame = rand() % 2000;
			int bounces = 2;

			PathTraceParameters rayParams = { UpdatedCamera, (int)frame, (int)reset, bounces };

			Context.CommandBuffer->BindRayTracingPipeline(PTPipeline);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(PTPipeline, 7, 1, &RTSet);
			Context.BindGPUScene(PTPipeline);

			Context.CommandBuffer->PushConstantsRayTracing(PTPipeline, ShaderType::Raygen, 0, sizeof(rayParams), &rayParams);
			Context.CommandBuffer->TraceRays(PTPipeline, 1280, 720, 1);
		};
	}

	void PathTraceDisplayPass::Setup(RenderGraphContext& Context)
	{
		GraphicsPipelineDesc Desc;
		Desc.Name = "Path Trace Display Shader";
		Desc.rasterizerState.Cull = CullMode::No;
		Desc.VS = std::make_shared<ShaderStage>(PathTraceDisplayVertexShader, "main", ShaderType::Vertex, ShaderLanguage::GLSL);
		Desc.PS = std::make_shared<ShaderStage>(PathTraceDisplayPixelShader, "main", ShaderType::Pixel, ShaderLanguage::GLSL);
		Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
	}

	RenderPass::TExecutionFunc PathTraceDisplayPass::Execute2(RenderGraphContext& Context)
	{
		TraceResult = Context.GetInputTexture(PathTracePass::RenderTargetName); // will synchronize with RT

		return [this](RenderGraphContext& Context)
		{
			if (Context.Scene->Dirty)
			{
				Frame = 1;
			}

			PathTraceDisplayParameters Params{Frame++};

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, TraceResult);

			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
			Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Pixel, 0, sizeof(Params), &Params);
			Context.CommandBuffer->Draw(3, 1, 0, 0);
		};
	}*/

}
