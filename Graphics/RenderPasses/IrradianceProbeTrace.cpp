#include "Core/Core.h"
#include "Core/CVar.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/GPUScene.h"
#include "Graphics/RenderGraph.h"
#include "Profiling/Profiling.h"
#include "RayTracingIrradianceVolumes.h"
#include "RenderPasses.h"
#include "imgui.h"
#include "Graphics/ShaderCache.h"

#include <algorithm>

namespace Columbus
{
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterIrradianceProbesApply);

	IMPLEMENT_GPU_PROFILING_COUNTER("Irradiance Probes Apply", "RenderGraphGPU", GpuCounterIrradianceProbesApply);

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

	struct IrradianceProbesApplyParameters
	{
		iVector2 Resolution{};
		iVector2 Padding{};
	};

	struct IrradianceVolumeTraceShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/IrradianceVolume/IrradianceVolumeTrace.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct PipelinePermutation
		{
			u32 MaxRecursionDepth = 1;
		};

		static RayTracingPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			RayTracingPipelineDesc Desc;
			Desc.Name = "IrradianceProbesTrace";
			Desc.MaxRecursionDepth = Permutation.MaxRecursionDepth;
			return Desc;
		}

		struct TraceParameters
		{
			ShaderGPUScene Scene;
			ShaderAccelerationStructure AccelerationStructure;
			ShaderReadBuffer IrradianceProbes;
			ShaderWriteBuffer IrradianceFixedRayDataBuffer;
			ShaderPushConstants<IrradianceProbesTraceParameters> Constants { {}, ShaderType::Raygen };
		};

		static void Bind(ShaderBinder& Binder, const TraceParameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.AccelerationStructure, 2, 0);
			Binder.Bind(Params.IrradianceProbes, 3, 0);
			Binder.Bind(Params.IrradianceFixedRayDataBuffer, 3, 1);
			Binder.Bind(Params.Constants);
		}

		struct AmbientParameters
		{
			ShaderGPUScene Scene;
			ShaderAccelerationStructure AccelerationStructure;
			ShaderWriteBuffer IrradianceProbes;
			ShaderReadBuffer IrradianceFixedRayDataBuffer;
			ShaderPushConstants<IrradianceProbesTraceParameters> Constants { {}, ShaderType::Raygen };
		};

		static void Bind(ShaderBinder& Binder, const AmbientParameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.AccelerationStructure, 2, 0);
			Binder.Bind(Params.IrradianceProbes, 3, 0);
			Binder.Bind(Params.IrradianceFixedRayDataBuffer, 3, 1);
			Binder.Bind(Params.Constants);
		}
	};

	struct IrradianceVolumeRelocateClassifyShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/IrradianceVolume/IrradianceVolumeRelocateClassify.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderWriteBuffer IrradianceProbes;
			ShaderReadBuffer IrradianceFixedRayDataBuffer;
			ShaderPushConstants<IrradianceProbesRelocateClassifyParameters> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.IrradianceProbes, 0, 0);
			Binder.Bind(Params.IrradianceFixedRayDataBuffer, 0, 1);
			Binder.Bind(Params.Constants);
		}
	};

	struct IrradianceProbesApplyShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/IrradianceVolume/IrradianceVolumeApply.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderConstants<RayTracingIrradianceVolumes::Constants> IrradianceVolumeConstants;
			std::array<ShaderReadBuffer, RayTracingIrradianceVolumes::MaxVolumes> IrradianceProbeBuffers;
			ShaderSampledTexture GBufferDepth { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture GBufferNormal;
			ShaderReadBuffer SceneBuffer;
			ShaderStorageTexture Output;
			ShaderPushConstants<IrradianceProbesApplyParameters> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.IrradianceVolumeConstants, 0, 9);
			ShaderArray<ShaderReadBuffer> IrradianceProbeBuffers;
			IrradianceProbeBuffers.Set(Params.IrradianceProbeBuffers.data(), RayTracingIrradianceVolumes::MaxVolumes);
			Binder.Bind(IrradianceProbeBuffers, 0, 10);
			Binder.Bind(Params.GBufferDepth, 1, 0);
			Binder.Bind(Params.GBufferNormal, 1, 1);
			Binder.Bind(Params.SceneBuffer, 1, 2);
			Binder.Bind(Params.Output, 1, 3);
			Binder.Bind(Params.Constants);
		}
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
			IrradianceVolumeTraceShader::TraceParameters TraceParams;
			TraceParams.Scene.UseCombinedSampler = false;
			TraceParams.IrradianceProbes = ProbesBuffer;
			TraceParams.IrradianceFixedRayDataBuffer = FixedRayBuffer;
			TraceParams.Constants.Value = {
				.SamplesPerProbe = 1,
				.Bounces = Bounces,
				.Random = 0,
				.Mode = 0,
				.RayDataStride = VisibilityRayCount,
				.ProbeOffset = ProbeOffset,
				.ProbeCount = ProbeCount,
				.SampleOffset = 0,
				.TotalSamplesPerProbe = 1,
			};

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<IrradianceVolumeTraceShader>(TraceParams);

			Graph.AddPass(Name, RenderGraphPassType::Compute, Parameters, Dependencies, [TraceParams, ProbeCount](RenderGraphContext& Context)
			{
				IrradianceVolumeTraceShader::TraceParameters Parameters = TraceParams;
				Parameters.AccelerationStructure = Context.Scene->TLAS;
				Parameters.Constants.Value.Random = rand();

				RayTracingPipeline* Pipeline = GetRayTracingPipeline<IrradianceVolumeTraceShader>(Context, IrradianceVolumeTraceShader::Permutation {}, IrradianceVolumeTraceShader::PipelinePermutation {});
				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindRayTracingParameters<IrradianceVolumeTraceShader>(Pipeline, Parameters);
				Context.CommandBuffer->TraceRays(Pipeline, 32, ProbeCount, 1);
			});
		};

		auto AddRelocateClassifyPass = [&](const char* Name, int Iteration)
		{
			IrradianceVolumeRelocateClassifyShader::Parameters RelocateParams;
			RelocateParams.IrradianceProbes = ProbesBuffer;
			RelocateParams.IrradianceFixedRayDataBuffer = FixedRayBuffer;
			RelocateParams.Constants.Value = {
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

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<IrradianceVolumeRelocateClassifyShader>(RelocateParams);

			Graph.AddPass(Name, RenderGraphPassType::Compute, Parameters, Dependencies, [RelocateParams, TotalProbes](RenderGraphContext& Context)
			{
				ComputePipeline* Pipeline = GetComputePipeline<IrradianceVolumeRelocateClassifyShader>(Context, IrradianceVolumeRelocateClassifyShader::Permutation {});
				Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<IrradianceVolumeRelocateClassifyShader>(Pipeline, RelocateParams);
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
			IrradianceVolumeTraceShader::TraceParameters TraceParams;
			TraceParams.Scene.UseCombinedSampler = false;
			TraceParams.IrradianceProbes = ProbesBuffer;
			TraceParams.IrradianceFixedRayDataBuffer = FixedRayBuffer;
			TraceParams.Constants.Value = {
				.SamplesPerProbe = VisibilityRayCount,
				.Bounces = Bounces,
				.Random = 0,
				.Mode = 2,
				.RayDataStride = VisibilityRayCount,
				.ProbeOffset = ProbeOffset,
				.ProbeCount = ProbeCount,
				.SampleOffset = 0,
				.TotalSamplesPerProbe = VisibilityRayCount,
			};

			RenderPassDependencies VisibilityTraceDependencies(Graph.Allocator);
			VisibilityTraceDependencies.Bind<IrradianceVolumeTraceShader>(TraceParams);

			Graph.AddPass("IrradianceProbesTraceVisibility", RenderGraphPassType::Compute, Parameters, VisibilityTraceDependencies, [TraceParams, VisibilityRayCount, ProbeCount](RenderGraphContext& Context)
			{
				IrradianceVolumeTraceShader::TraceParameters Parameters = TraceParams;
				Parameters.AccelerationStructure = Context.Scene->TLAS;
				Parameters.Constants.Value.Random = rand();

				RayTracingPipeline* Pipeline = GetRayTracingPipeline<IrradianceVolumeTraceShader>(Context, IrradianceVolumeTraceShader::Permutation {}, IrradianceVolumeTraceShader::PipelinePermutation {});
				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindRayTracingParameters<IrradianceVolumeTraceShader>(Pipeline, Parameters);
				Context.CommandBuffer->TraceRays(Pipeline, VisibilityRayCount, ProbeCount, 1);
			});
		};

		for (int ProbeOffset = 0; ProbeOffset < TotalProbes; ProbeOffset += ProbeChunkSize)
		{
			AddVisibilityTracePass(ProbeOffset, std::min(ProbeChunkSize, TotalProbes - ProbeOffset));
		}

		IrradianceVolumeRelocateClassifyShader::Parameters VisibilityBlendParams;
		VisibilityBlendParams.IrradianceProbes = ProbesBuffer;
		VisibilityBlendParams.IrradianceFixedRayDataBuffer = FixedRayBuffer;
		VisibilityBlendParams.Constants.Value = {
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

		RenderPassDependencies VisibilityBlendDependencies(Graph.Allocator);
		VisibilityBlendDependencies.Bind<IrradianceVolumeRelocateClassifyShader>(VisibilityBlendParams);

		Graph.AddPass("IrradianceProbesBlendVisibility", RenderGraphPassType::Compute, Parameters, VisibilityBlendDependencies, [VisibilityBlendParams, TotalProbes](RenderGraphContext& Context)
		{
			ComputePipeline* Pipeline = GetComputePipeline<IrradianceVolumeRelocateClassifyShader>(Context, IrradianceVolumeRelocateClassifyShader::Permutation {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<IrradianceVolumeRelocateClassifyShader>(Pipeline, VisibilityBlendParams);
			Context.CommandBuffer->Dispatch((TotalProbes * 64 + 63) / 64, 1, 1);
		});

		auto AddAmbientTracePass = [&](int ProbeOffset, int ProbeCount, int SampleOffset, int SamplesThisPass)
		{
			IrradianceVolumeTraceShader::AmbientParameters AmbientParams;
			AmbientParams.Scene.UseCombinedSampler = false;
			AmbientParams.IrradianceProbes = ProbesBuffer;
			AmbientParams.IrradianceFixedRayDataBuffer = FixedRayBuffer;
			AmbientParams.Constants.Value = {
				.SamplesPerProbe = SamplesThisPass,
				.Bounces = Bounces,
				.Random = 0,
				.Mode = 1,
				.RayDataStride = VisibilityRayCount,
				.ProbeOffset = ProbeOffset,
				.ProbeCount = ProbeCount,
				.SampleOffset = SampleOffset,
				.TotalSamplesPerProbe = RaysPerProbe,
			};

			RenderPassDependencies AmbientDependencies(Graph.Allocator);
			AmbientDependencies.Bind<IrradianceVolumeTraceShader>(AmbientParams);

			Graph.AddPass("IrradianceProbesTraceAmbientCube", RenderGraphPassType::Compute, Parameters, AmbientDependencies, [AmbientParams, ProbeCount](RenderGraphContext& Context)
			{
				IrradianceVolumeTraceShader::AmbientParameters Parameters = AmbientParams;
				Parameters.AccelerationStructure = Context.Scene->TLAS;
				Parameters.Constants.Value.Random = rand();

				RayTracingPipeline* Pipeline = GetRayTracingPipeline<IrradianceVolumeTraceShader>(Context, IrradianceVolumeTraceShader::Permutation {}, IrradianceVolumeTraceShader::PipelinePermutation {});
				Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
				Context.BindRayTracingParameters<IrradianceVolumeTraceShader>(Pipeline, Parameters);
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

	void RenderApplyIrradianceProbes(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, const RayTracingIrradianceVolumes::Prepared& IrradianceVolumes)
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

		if (IrradianceVolumes.Constants.Value.CountAndFlags.X == 0)
		{
			return;
		}

		IrradianceProbesApplyShader::Parameters ApplyParams;
		ApplyParams.IrradianceVolumeConstants = IrradianceVolumes.Constants;
		ApplyParams.IrradianceProbeBuffers = IrradianceVolumes.ProbeBuffers;
		ApplyParams.GBufferDepth = Textures.GBufferDS;
		ApplyParams.GBufferNormal = Textures.GBufferNormal;
		ApplyParams.SceneBuffer = Graph.Scene->SceneBuffer;
		ApplyParams.Output = GI_Tex;
		ApplyParams.Constants.Value = {
			.Resolution = Size,
			.Padding = iVector2(0, 0),
		};

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<IrradianceProbesApplyShader>(ApplyParams);

		// TODO: convert to optimised raster
		Graph.AddPass("IrradianceProbesApply", RenderGraphPassType::Compute, Parameters, Dependencies, [ApplyParams, Size](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterIrradianceProbesApply, Context);

			ComputePipeline* Pipeline = GetComputePipeline<IrradianceProbesApplyShader>(Context, IrradianceProbesApplyShader::Permutation {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<IrradianceProbesApplyShader>(Pipeline, ApplyParams);
			Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { Size, 1 });
		});
	}

}
