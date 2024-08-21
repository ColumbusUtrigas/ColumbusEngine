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
		// alignas(16) Vector3 test;
		alignas(16) Vector4 Irradiance[6]; // 112

		// alignas(16) Vector4 FullIrradiance[64];
		// alignas(16) Vector4 Directions[64];
	};

	struct IrradianceProbesTraceParameters
	{
		int RaysPerProbe;
		int Bounces;
		int Random;
	};

	void RenderIrradianceProbes(RenderGraph& Graph, const RenderView& View, IrradianceVolume& Volume)
	{
		if (!Graph.Device->SupportsRayTracing())
		{
			return;
		}

		// TODO: invent something better than this
		std::vector<GPUIrradianceProbe> IrradianceProbes;

		for (int i = 0; i < Volume.ProbesCount.X; i++)
		{
			for (int j = 0; j < Volume.ProbesCount.Y; j++)
			{
				for (int k = 0; k < Volume.ProbesCount.Z; k++)
				{
					Vector3 CellSize = Volume.Extent / Vector3(Volume.ProbesCount);
					Vector3 ProbePosition = Vector3((float)i, (float)j, (float)k) * CellSize + CellSize/2 - Volume.Extent/2 + Volume.Position;
					IrradianceProbes.push_back(GPUIrradianceProbe{ProbePosition});
				}
			}
		}

		BufferDesc ProbeBufferDesc;
		ProbeBufferDesc.BindFlags = BufferType::UAV;
		ProbeBufferDesc.Size = Volume.GetTotalProbes() * sizeof(GPUIrradianceProbe);
		Volume.ProbesBuffer = Graph.Device->CreateBuffer(ProbeBufferDesc, IrradianceProbes.data());

		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies(Graph.Allocator);

		Graph.AddPass("IrradianceProbesTrace", RenderGraphPassType::Compute, Parameters, Dependencies, [Volume](RenderGraphContext& Context)
		{
			static RayTracingPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				RayTracingPipelineDesc Desc{};
				Desc.Name = "IrradianceProbesTrace";
				Desc.MaxRecursionDepth = 1;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/IrradianceVolume/IrradianceVolumeTrace.csd");
				Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
			}

			auto TlasSet = Context.GetDescriptorSet(Pipeline, 2);
			auto ProbesSet = Context.GetDescriptorSet(Pipeline, 3);

			Context.Device->UpdateDescriptorSet(TlasSet, 0, 0, Context.Scene->TLAS);
			Context.Device->UpdateDescriptorSet(ProbesSet, 0, 0, Volume.ProbesBuffer);

			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &TlasSet);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 3, 1, &ProbesSet);
			Context.BindGPUScene(Pipeline, false);

			IrradianceProbesTraceParameters Parameters {
				.RaysPerProbe = 512, // 128
				.Bounces = 3,
				.Random = rand(),
			};

			Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Parameters), &Parameters);
			Context.CommandBuffer->TraceRays(Pipeline, Volume.GetTotalProbes(), 1, 1);
		});
	}

	void RenderApplyIrradianceProbes(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, IrradianceVolume& Volume)
	{
		iVector2 Size = View.RenderSize;

		TextureDesc2 Desc{
			.Usage = TextureUsage::Storage | TextureUsage::Sampled,
			.Width = (u32)Size.X,
			.Height = (u32)Size.Y,
			.Format = TextureFormat::RGBA16F,
		};

		RenderGraphTextureRef GI_Tex = Graph.CreateTexture(Desc, "IrradianceVolumeOutput");

		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(GI_Tex, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Textures.RTGI = GI_Tex;

		if (Volume.ProbesBuffer != nullptr)
		{
			Graph.AddPass("IrradianceProbesApply", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, GI_Tex, Volume, Size](RenderGraphContext& Context)
			{
				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "IV_Apply";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/IrradianceVolume/IrradianceVolumeApply.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto Set = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(Set, 0, 0, Volume.ProbesBuffer);

				auto Set2 = Context.GetDescriptorSet(Pipeline, 1);
				Context.Device->UpdateDescriptorSet(Set2, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set2, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set2, 2, 0, Context.GetRenderGraphTexture(GI_Tex).get());

				struct
				{
					// TODO: move to appropriate place
					Vector4 Position;
					Vector4 Extent;
					iVector4 ProbesCount;
					iVector4 ProbeIndex;

					iVector2 Resolution;
				} Params;

				Params.Position = Vector4(Volume.Position, 0);
				Params.Extent = Vector4(Volume.Extent, 0);
				Params.ProbesCount = iVector4(Volume.ProbesCount, 0);
				Params.ProbeIndex = iVector4(Volume.Extent, 0);
				Params.Resolution = Size;

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 1, 1, &Set2);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { Size, 1 });
			});
		}
	}

}
