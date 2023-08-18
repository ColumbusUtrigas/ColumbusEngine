#include "Core/Core.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/GPUScene.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"
#include "imgui.h"

namespace Columbus
{

	struct GPUIrradianceProbe
	{
		alignas(16) Vector3 Position; // 16
		alignas(16) Vector3 test;
		alignas(16) Vector4 Irradiance[6]; // 112

		alignas(16) Vector4 FullIrradiance[64];
		alignas(16) Vector4 Directions[64];
	};

	std::vector<GPUIrradianceProbe> IrradianceProbes;

	struct Parameters
	{
		int RaysPerProbe;
		int Bounces;
		int Frame;
	};

	void IrradianceProbeTracePass::Setup(RenderGraphContext& Context)
	{
		auto ShaderSource = LoadShaderFile("IrradianceProbesTrace.glsl");

		RayTracingPipelineDesc Desc{};
		Desc.Name = "RTXON";
		Desc.RayGen = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Raygen, ShaderLanguage::GLSL);
		Desc.Miss = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::Miss, ShaderLanguage::GLSL);
		Desc.ClosestHit = std::make_shared<ShaderStage>(ShaderSource, "main", ShaderType::ClosestHit, ShaderLanguage::GLSL);
		Desc.MaxRecursionDepth = 1;
		Pipeline = Context.Device->CreateRayTracingPipeline(Desc);

		for (int i = 0; i < Volume.ProbesCount.X; i++)
		{
			for (int j = 0; j < Volume.ProbesCount.Y; j++)
			{
				for (int k = 0; k < Volume.ProbesCount.Z; k++)
				{
					IrradianceProbes.push_back(GPUIrradianceProbe{Vector3(1000, 50, -250) + Vector3(i * 50, k * 80, j * 100)});
				}
			}
		}
	}

	RenderPass::TExecutionFunc IrradianceProbeTracePass::Execute2(RenderGraphContext& Context)
	{
		static int RaysPerProbe = 64;
		static int Bounces = 1;
		static int Frame = 0;

		return [this](RenderGraphContext& Context)
		{
			// ImGui::Begin("IrradianceProbeTrace Options");
			// ImGui::SliderInt("Rays per probe", &RaysPerProbe, 16, 512);
			// ImGui::SliderInt("Bounces", &Bounces, 0, 4);
			// ImGui::End();

			BufferDesc ProbeBufferDesc;
			ProbeBufferDesc.BindFlags = BufferType::UAV;
			ProbeBufferDesc.Size = IrradianceProbes.size() * sizeof(GPUIrradianceProbe);
			Buffer* IrradianceProbeBuffer = Context.GetOutputBuffer(ProbeBufferName, ProbeBufferDesc, IrradianceProbes.data());

			auto RTSet = Context.GetDescriptorSet(Pipeline, 7);
			Context.Device->UpdateDescriptorSet(RTSet, 0, 0, Context.Scene->TLAS); // TODO: move to unified scene set

			auto ProbeSet = Context.GetDescriptorSet(Pipeline, 8);
			Context.Device->UpdateDescriptorSet(ProbeSet, 0, 0, IrradianceProbeBuffer);

			Parameters Params = { RaysPerProbe, Bounces, Frame % 100 };
			// Frame++;

			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 7, 1, &RTSet);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 8, 1, &ProbeSet);
			Context.BindGPUScene(Pipeline);

			Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Params), &Params);
			Context.CommandBuffer->TraceRays(Pipeline, IrradianceProbes.size(), 1, 1);
		};
	}

}
