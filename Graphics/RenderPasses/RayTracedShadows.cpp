#include "Common/Image/Image.h"
#include "Core/CVar.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"
#include "imgui.h"

namespace Columbus
{

	ConsoleVariable<float> Cvar_Angle("r.RTShadows.Angle", "Controls angle of randomisation of shadow rays", 0.0f);

	struct RTShadowParams
	{
		Vector3 Direction;
		float Angle;
		float Random;
		bool ValidHistory;
	};

	RenderGraphTextureRef RayTracedShadowsPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures)
	{
		// TODO: denoising with history

		static Vector3 LightDirection(1,1,1);
		static float LightRadius = 1;

		if (ImGui::Begin("Light"))
		{
			ImGui::SliderFloat3("Direction", (float*)&LightDirection, -1, 1);
			ImGui::SliderFloat("Radius", &LightRadius, 0, 5);
		}
		ImGui::End();

		TextureDesc2 Desc {
			.Usage = TextureUsage::Storage,
			.Width = (u32)View.OutputSize.X,
			.Height = (u32)View.OutputSize.Y,
			.Format = TextureFormat::R8,
		};
		RenderGraphTextureRef RTShadow = Graph.CreateTexture(Desc, "RayTracedShadow");

		// shouldn't be static, lifetime must be defined
		static SPtr<Texture2> History;

		// fallback?
		// RenderGraphTextureRef RTShadowHistory = Graph.RegisterExternalTexture(History, "RayTracedShadowHistory");

		{
			RenderPassParameters Parameters;

			RenderPassDependencies Dependencies;
			Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(RTShadow, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			// Dependencies.Write(RTShadowHistory);

			Graph.AddPass("RayTraceShadow", RenderGraphPassType::Compute, Parameters, Dependencies, [RTShadow, Textures, View](RenderGraphContext& Context)
			{
				static RayTracingPipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					RayTracingPipelineDesc Desc;
					Desc.Name = "RayTracedShadowsPass";
					Desc.MaxRecursionDepth = 1;
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RayTracedShadowsPass.csd");

					Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
				}

				GPUCamera UpdatedCamera = GPUCamera(View.Camera);
				Context.Scene->Dirty = Context.Scene->MainCamera != UpdatedCamera; // TODO: move to the main rendering system
				Context.Scene->MainCamera = UpdatedCamera;

				auto ShadowsBufferSet = Context.GetDescriptorSet(Pipeline, 7);
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 0, 0, Context.Scene->TLAS);
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 1, 0, Context.GetRenderGraphTexture(RTShadow).get());
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 2, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get());
				Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 3, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get());

				if (History)
					Context.Device->UpdateDescriptorSet(ShadowsBufferSet, 4, 0, History.get());

				// RTShadowParams Params { .Angle = Cvar_Angle.GetValue() };
				RTShadowParams Params {
					.Direction = LightDirection, .Angle = LightRadius,
					.Random = (rand() % 2000) / 2000.0f,
					.ValidHistory = History != nullptr && !Context.Scene->Dirty
				};

				Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Params), &Params);
				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindGPUScene(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 7, 1, &ShadowsBufferSet);

				Context.CommandBuffer->TraceRays(Pipeline, View.OutputSize.X, View.OutputSize.Y, 1);
			});
		}

		// creates/recreates image and copies it to be used in the next frame
		Graph.ExtractTexture(RTShadow, &History);

		return RTShadow;
	}

}
