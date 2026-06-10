#include "Core/Core.h"
#include "Core/CVar.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/GPUScene.h"
#include "Graphics/RenderGraph.h"
#include "RayTracingIrradianceVolumes.h"
#include "RenderPasses.h"
#include "imgui.h"

#include <algorithm>

namespace Columbus
{

	// Roadmap:
	// - Resources definition and tooling/workflow preparation goes first
	// - Allocation process
	// - Baking into spherical harmonics or octohedrals
	// - Saving process (level lighting data)
	// - Loading process from the level lighting data
	// - Efficient rendering
	// - Data compression BC6H
	// - Dynamicity support - turning off light sources, changing weather params, changing scene lighting setups

	struct IrradianceProbesTraceParameters
	{
		int SamplesPerProbe;
		int Bounces;
		int Random;
		int Mode;
		int RayDataStride;
		int ProbeOffset;
		int ProbeCount;
		int SampleOffset;
		int TotalSamplesPerProbe;
	};

	struct IrradianceProbesRelocateClassifyParameters
	{
		Vector4 Position;
		Vector4 Extent;
		iVector4 ProbesCount;
		int TotalProbes;
		int FixedRayCount;
		int Iteration;
		int Mode;
		int RayDataStride;
		int VisibilityRayCount;
	};

	ConsoleVariable<int> CVar_IrradianceVolumeBakeProbeChunk("r.IrradianceVolume.BakeProbeChunk", "Maximum probes processed by one irradiance volume bake dispatch", 512);
	ConsoleVariable<int> CVar_IrradianceVolumeBakeSampleChunk("r.IrradianceVolume.BakeSampleChunk", "Maximum rays per probe lobe accumulated by one irradiance volume ambient bake dispatch", 32768);

	void RenderIrradianceProbes(RenderGraph& Graph, const RenderView& View, IrradianceVolume& Volume, int RaysPerProbe, int Bounces)
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
					GPUIrradianceProbe Probe{};
					Probe.PositionState = Vector4(ProbePosition, 1.0f);
					IrradianceProbes.push_back(Probe);
				}
			}
		}

		if (Volume.ProbesBuffer != nullptr)
		{
			RemoveProfilingMemory(MemoryCounter_SceneIrradianceProbes, Volume.ProbesBufferBytes);
			Volume.ProbesBufferBytes = 0;
			Graph.Device->DestroyBufferDeferred(Volume.ProbesBuffer);
			Volume.ProbesBuffer = nullptr;
		}

		BufferDesc ProbeBufferDesc;
		ProbeBufferDesc.BindFlags = BufferType::UAV;
		ProbeBufferDesc.Size = Volume.GetTotalProbes() * sizeof(GPUIrradianceProbe);
		Volume.ProbesBuffer = Graph.Device->CreateBuffer(ProbeBufferDesc, IrradianceProbes.data());
		Volume.ProbesBufferBytes = Volume.ProbesBuffer->GetSize();
		AddProfilingMemory(MemoryCounter_SceneIrradianceProbes, Volume.ProbesBufferBytes);

		RenderPassParameters Parameters;
		const int TotalProbes = Volume.GetTotalProbes();
		const int FixedRayCount = 32;
		const int VisibilityRayCount = 128;
		const int RelocationIterations = 5;
		const int ProbeChunkSize = std::max(1, CVar_IrradianceVolumeBakeProbeChunk.GetValue());
		const int SampleChunkSize = std::max(1, CVar_IrradianceVolumeBakeSampleChunk.GetValue());
		RenderGraphBufferRef ProbesBuffer = Graph.RegisterExternalBuffer(Volume.ProbesBuffer, "IrradianceProbes");

		BufferDesc FixedRayBufferDesc;
		FixedRayBufferDesc.BindFlags = BufferType::UAV;
		FixedRayBufferDesc.Size = TotalProbes * VisibilityRayCount * sizeof(float);
		RenderGraphBufferRef FixedRayBuffer = Graph.CreateBuffer(FixedRayBufferDesc, "IrradianceProbeFixedRayData");

		auto AddFixedRayTracePass = [&](const char* Name, int ProbeOffset, int ProbeCount)
		{
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.ReadBuffer(ProbesBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.WriteBuffer(FixedRayBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass(Name, RenderGraphPassType::Compute, Parameters, Dependencies, [ProbesBuffer, FixedRayBuffer, Bounces, ProbeOffset, ProbeCount](RenderGraphContext& Context)
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
			Context.Device->UpdateDescriptorSet(ProbesSet, 0, 0, Context.GetRenderGraphBuffer(ProbesBuffer).get());
			Context.Device->UpdateDescriptorSet(ProbesSet, 1, 0, Context.GetRenderGraphBuffer(FixedRayBuffer).get());

			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &TlasSet);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 3, 1, &ProbesSet);
			Context.BindGPUScene(Pipeline, false);

			IrradianceProbesTraceParameters Parameters {
				.SamplesPerProbe = 1,
				.Bounces = Bounces,
				.Random = rand(),
				.Mode = 0,
				.RayDataStride = VisibilityRayCount,
				.ProbeOffset = ProbeOffset,
				.ProbeCount = ProbeCount,
				.SampleOffset = 0,
				.TotalSamplesPerProbe = 1,
			};

			Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Parameters), &Parameters);
			Context.CommandBuffer->TraceRays(Pipeline, 32, ProbeCount, 1);
			});
		};

		auto AddRelocateClassifyPass = [&](const char* Name, int Iteration)
		{
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.ReadBuffer(FixedRayBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.ReadBuffer(ProbesBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			Dependencies.WriteBuffer(ProbesBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			Graph.AddPass(Name, RenderGraphPassType::Compute, Parameters, Dependencies, [ProbesBuffer, FixedRayBuffer, Volume, TotalProbes, FixedRayCount, Iteration](RenderGraphContext& Context)
			{
				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "IV_RelocateClassify";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/IrradianceVolume/IrradianceVolumeRelocateClassify.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto Set = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphBuffer(ProbesBuffer).get());
				Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphBuffer(FixedRayBuffer).get());

				IrradianceProbesRelocateClassifyParameters Params{
					.Position = Vector4(Volume.Position, 0),
					.Extent = Vector4(Volume.Extent, 0),
					.ProbesCount = iVector4(Volume.ProbesCount, 0),
					.TotalProbes = TotalProbes,
					.FixedRayCount = FixedRayCount,
					.Iteration = Iteration,
					.Mode = 0,
					.RayDataStride = VisibilityRayCount,
					.VisibilityRayCount = VisibilityRayCount,
				};

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.CommandBuffer->Dispatch((TotalProbes + 63) / 64, 1, 1);
			});
		};

		for (int Iteration = 0; Iteration < RelocationIterations; Iteration++)
		{
			for (int ProbeOffset = 0; ProbeOffset < TotalProbes; ProbeOffset += ProbeChunkSize)
			{
				AddFixedRayTracePass("IrradianceProbesTraceFixed", ProbeOffset, std::min(ProbeChunkSize, TotalProbes - ProbeOffset));
			}
			AddRelocateClassifyPass("IrradianceProbesRelocateClassify", Iteration);
		}

		auto AddVisibilityTracePass = [&](int ProbeOffset, int ProbeCount)
		{
			RenderPassDependencies VisibilityTraceDependencies(Graph.Allocator);
			VisibilityTraceDependencies.ReadBuffer(ProbesBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			VisibilityTraceDependencies.WriteBuffer(FixedRayBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass("IrradianceProbesTraceVisibility", RenderGraphPassType::Compute, Parameters, VisibilityTraceDependencies, [ProbesBuffer, FixedRayBuffer, Bounces, VisibilityRayCount, ProbeOffset, ProbeCount](RenderGraphContext& Context)
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
				Context.Device->UpdateDescriptorSet(ProbesSet, 0, 0, Context.GetRenderGraphBuffer(ProbesBuffer).get());
				Context.Device->UpdateDescriptorSet(ProbesSet, 1, 0, Context.GetRenderGraphBuffer(FixedRayBuffer).get());

				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &TlasSet);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 3, 1, &ProbesSet);
				Context.BindGPUScene(Pipeline, false);

				IrradianceProbesTraceParameters Parameters {
					.SamplesPerProbe = VisibilityRayCount,
					.Bounces = Bounces,
					.Random = rand(),
					.Mode = 2,
					.RayDataStride = VisibilityRayCount,
					.ProbeOffset = ProbeOffset,
					.ProbeCount = ProbeCount,
					.SampleOffset = 0,
					.TotalSamplesPerProbe = VisibilityRayCount,
				};

				Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Parameters), &Parameters);
				Context.CommandBuffer->TraceRays(Pipeline, VisibilityRayCount, ProbeCount, 1);
			});
		};

		for (int ProbeOffset = 0; ProbeOffset < TotalProbes; ProbeOffset += ProbeChunkSize)
		{
			AddVisibilityTracePass(ProbeOffset, std::min(ProbeChunkSize, TotalProbes - ProbeOffset));
		}

		RenderPassDependencies VisibilityBlendDependencies(Graph.Allocator);
		VisibilityBlendDependencies.ReadBuffer(FixedRayBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		VisibilityBlendDependencies.ReadBuffer(ProbesBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		VisibilityBlendDependencies.WriteBuffer(ProbesBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("IrradianceProbesBlendVisibility", RenderGraphPassType::Compute, Parameters, VisibilityBlendDependencies, [ProbesBuffer, FixedRayBuffer, Volume, TotalProbes, FixedRayCount, VisibilityRayCount](RenderGraphContext& Context)
		{
			static ComputePipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				ComputePipelineDesc Desc;
				Desc.Name = "IV_RelocateClassify";
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/IrradianceVolume/IrradianceVolumeRelocateClassify.csd");
				Pipeline = Context.Device->CreateComputePipeline(Desc);
			}

			auto Set = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphBuffer(ProbesBuffer).get());
			Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphBuffer(FixedRayBuffer).get());

			IrradianceProbesRelocateClassifyParameters Params{
				.Position = Vector4(Volume.Position, 0),
				.Extent = Vector4(Volume.Extent, 0),
				.ProbesCount = iVector4(Volume.ProbesCount, 0),
				.TotalProbes = TotalProbes,
				.FixedRayCount = FixedRayCount,
				.Iteration = 0,
				.Mode = 1,
				.RayDataStride = VisibilityRayCount,
				.VisibilityRayCount = VisibilityRayCount,
			};

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
			Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
			Context.CommandBuffer->Dispatch((TotalProbes * 64 + 63) / 64, 1, 1);
		});

		auto AddAmbientTracePass = [&](int ProbeOffset, int ProbeCount, int SampleOffset, int SamplesThisPass)
		{
			RenderPassDependencies AmbientDependencies(Graph.Allocator);
			AmbientDependencies.ReadBuffer(ProbesBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			AmbientDependencies.WriteBuffer(ProbesBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			AmbientDependencies.ReadBuffer(FixedRayBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass("IrradianceProbesTraceAmbientCube", RenderGraphPassType::Compute, Parameters, AmbientDependencies, [ProbesBuffer, FixedRayBuffer, RaysPerProbe, Bounces, ProbeOffset, ProbeCount, SampleOffset, SamplesThisPass](RenderGraphContext& Context)
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
				Context.Device->UpdateDescriptorSet(ProbesSet, 0, 0, Context.GetRenderGraphBuffer(ProbesBuffer).get());
				Context.Device->UpdateDescriptorSet(ProbesSet, 1, 0, Context.GetRenderGraphBuffer(FixedRayBuffer).get());

				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &TlasSet);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 3, 1, &ProbesSet);
				Context.BindGPUScene(Pipeline, false);

				IrradianceProbesTraceParameters Parameters {
					.SamplesPerProbe = SamplesThisPass,
					.Bounces = Bounces,
					.Random = rand(),
					.Mode = 1,
					.RayDataStride = VisibilityRayCount,
					.ProbeOffset = ProbeOffset,
					.ProbeCount = ProbeCount,
					.SampleOffset = SampleOffset,
					.TotalSamplesPerProbe = RaysPerProbe,
				};

				Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Parameters), &Parameters);
				Context.CommandBuffer->TraceRays(Pipeline, ProbeCount, 1, 1);
			});
		};

		for (int SampleOffset = 0; SampleOffset < RaysPerProbe; SampleOffset += SampleChunkSize)
		{
			const int SamplesThisPass = std::min(SampleChunkSize, RaysPerProbe - SampleOffset);
			for (int ProbeOffset = 0; ProbeOffset < TotalProbes; ProbeOffset += ProbeChunkSize)
			{
				AddAmbientTracePass(ProbeOffset, std::min(ProbeChunkSize, TotalProbes - ProbeOffset), SampleOffset, SamplesThisPass);
			}
		}
	}

	void RenderApplyIrradianceProbes(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures)
	{
		iVector2 Size = View.RenderSize;

		TextureDesc2 Desc{
			.Usage = TextureUsage::Storage | TextureUsage::Sampled,
			.Width = (u32)Size.X,
			.Height = (u32)Size.Y,
			.Format = TextureFormat::RGBA16F,
		};

		RenderGraphTextureRef GI_Tex = Graph.CreateTexture(Desc, "IrradianceVolumeOutput");

		ShaderMemsetTexture(Graph, GI_Tex, Vector4(0, 0, 0, 0));

		RenderPassParameters Parameters;

		Textures.RTGI = GI_Tex;

		RenderPassDependencies Dependencies(Graph.Allocator);
		RayTracingIrradianceVolumes::Prepared IrradianceVolumes = RayTracingIrradianceVolumes::Prepare(Graph, Dependencies, true, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		if (IrradianceVolumes.ProbeBuffers.empty())
		{
			return;
		}

		Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(GI_Tex, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("IrradianceProbesApply", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, GI_Tex, Size, IrradianceVolumes](RenderGraphContext& Context)
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
			RayTracingIrradianceVolumes::Bind(Context, Set, IrradianceVolumes, Context.Scene->SceneBuffer);

			auto Set2 = Context.GetDescriptorSet(Pipeline, 1);
			Context.Device->UpdateDescriptorSet(Set2, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set2, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set2, 2, 0, Context.GetRenderGraphTexture(GI_Tex).get());

			struct
			{
				iVector2 Resolution;
				iVector2 Padding;
			} Params;

			Params.Resolution = Size;
			Params.Padding = iVector2(0, 0);

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 1, 1, &Set2);
			Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
			Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { Size, 1 });
		});
	}

}
