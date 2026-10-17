#include "RadianceCache.h"
#include "RenderPasses.h"
#include "Core/CVar.h"
#include "Graphics/ShaderCache.h"
#include "Graphics/GPUScene.h"
#include "Profiling/Profiling.h"
#include "System/Log.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <span>

namespace Columbus::RadianceCache
{
	IMPLEMENT_CPU_PROFILING_COUNTER("Radiance cache preparation", "Render", CpuCounterRadianceCache);
	IMPLEMENT_GPU_PROFILING_COUNTER("SHaRC update", "RenderGraphGPU", GpuCounterRadianceCacheUpdate);
	IMPLEMENT_GPU_PROFILING_COUNTER("SHaRC resolve", "RenderGraphGPU", GpuCounterRadianceCacheResolve);
	IMPLEMENT_MEMORY_PROFILING_COUNTER("SHaRC resources", "RadianceCache", MemoryCounterRadianceCache);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("SHaRC history resets", "RadianceCache", CountRadianceCacheResets, true);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("SHaRC resource allocations", "RadianceCache", CountRadianceCacheAllocations, true);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("SHaRC update paths", "RadianceCache", CountRadianceCacheUpdatePaths, true);

	ConsoleVariable<int> CVar_ReflectionLightingMode("r.RTReflection.LightingMode", "Reflection lighting: 0 hit lighting, 1 cached diffuse lighting (no visibility rays; missing lighting is black)", 1);
	ConsoleVariable<bool> CVar_Enable("r.RadianceCache.Enable", "Enable SHaRC for effects with UseRadianceCache enabled", true);
	ConsoleVariable<int> CVar_Entries("r.RadianceCache.EntryCount", "SHaRC capacity, rounded to a power of two (65536..4194304)", 1048576);
	ConsoleVariable<int> CVar_UpdatePathBudget("r.RadianceCache.UpdatePathBudget", "Maximum SHaRC update paths per frame (1..1048576); small views use fewer paths", 10000);
	ConsoleVariable<int> CVar_Bounces("r.RadianceCache.Bounces", "Maximum secondary vertices per SHaRC update path (1..8)", 3);
	ConsoleVariable<int> CVar_AccumulationFrames("r.RadianceCache.AccumulationFrames", "SHaRC temporal accumulation window (1..64)", 32);
	ConsoleVariable<int> CVar_StaleFrames("r.RadianceCache.StaleFrames", "Evict cache entries without updates after this many frames (8..128)", 64);
	ConsoleVariable<float> CVar_SceneScale("r.RadianceCache.SceneScale", "SHaRC spatial detail: larger values produce smaller cells (1..256)", 16.0f);
	ConsoleVariable<int> CVar_Reset("r.RadianceCache.Reset", "Change this generation to clear SHaRC without reallocating GPU resources", 0);
	ConsoleVariable<bool> CVar_Statistics("r.RadianceCache.Statistics", "Collect cached-lighting GPU query/hit/miss counts in SHaRC Statistics", false);
	ConsoleVariable<int> CVar_Visualisation("r.RadianceCache.Visualisation", "RadianceCache view: 0 cached radiance, 1 hash cells, 2 sample count", 0);

	static constexpr u32 HashStride         = 8;
	static constexpr u32 AccumulationStride = 32;
	static constexpr u32 ResolvedStride     = 24;
	static constexpr const char* BufferNames[] = { "SHaRC Hash entries", "SHaRC Accumulation", "SHaRC Resolved", "SHaRC Statistics" };

	bool IsReflectionCacheRequested(const RenderView& View)
	{
		return View.DeferredSettings.RTReflectionsUseRadianceCache && std::clamp(CVar_ReflectionLightingMode.GetValue(), 0, 1) == 1;
	}

	void QueryParameters::Bind(ShaderBinder& Binder, int Set) const
	{
		if (!Enabled)
		{
			return;
		}

		Binder.Bind(HashEntries, Set, 0);
		Binder.Bind(Accumulation, Set, 1);
		Binder.Bind(Resolved, Set, 2);
		Binder.Bind(Constants, Set, 3);
		Binder.Bind(Statistics, Set, 4);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Persistent resources

	void State::Destroy(SPtr<DeviceVulkan> Device)
	{
		if (Buffers[0] != nullptr)
		{
			Device->QueueWaitIdle();
		}

		for (Buffer*& Buffer : Buffers)
		{
			if (Buffer != nullptr)
			{
				RemoveProfilingMemory(MemoryCounterRadianceCache, Buffer->GetSize());
				Device->DestroyBuffer(Buffer);
				Buffer = nullptr;
			}
		}

		EntryCount = 0;
		SceneIdentity = nullptr;
		ActiveLastFrame = false;
	}

	static bool EnsureResources(RenderGraph& Graph, State& Cache, u32 EntryCount)
	{
		if (Cache.EntryCount == EntryCount)
		{
			return false;
		}

		Cache.Destroy(Graph.Device);
		const u64 Sizes[] = { u64(EntryCount) * HashStride, u64(EntryCount) * AccumulationStride, u64(EntryCount) * ResolvedStride, 4 * sizeof(u32) };

		for (u32 i = 0; i < Cache.Buffers.size(); ++i)
		{
			BufferDesc Desc(Sizes[i], BufferType::UAV);
			Cache.Buffers[i] = Graph.Device->CreateBuffer(Desc, nullptr);
			Graph.Device->SetDebugName(Cache.Buffers[i], BufferNames[i]);
			AddProfilingMemory(MemoryCounterRadianceCache, Cache.Buffers[i]->GetSize());
		}

		Cache.EntryCount = EntryCount;
		Cache.FrameIndex = 0;
		return true;
	}

	// Structural/material changes invalidate cached surfaces. Runtime light/sky changes
	// converge through temporal updates; hashing them here would flush every frame for
	// a moving flashlight, flickering light, or camera-dependent effects-volume blend.
	static u64 ContentSignature(const GPUScene& Scene)
	{
		u64 Signature = 14695981039346656037ull;
		auto Add = [&](const void* Data, size_t Size)
		{
			const auto* Bytes = static_cast<const u8*>(Data);

			for (size_t i = 0; i < Size; ++i)
			{
				Signature = (Signature ^ Bytes[i]) * 1099511628211ull;
			}
		};

		for (const Material& Material : std::span(Scene.Materials.Data(), Scene.Materials.Size()))
		{
			Add(&Material.AlbedoFactor, sizeof(Material.AlbedoFactor));
			Add(&Material.EmissiveFactor, sizeof(Material.EmissiveFactor));
			Add(&Material.Roughness, sizeof(Material.Roughness));
			Add(&Material.Metallic, sizeof(Material.Metallic));
			Add(&Material.AlbedoId, sizeof(Material.AlbedoId));
			Add(&Material.OrmId, sizeof(Material.OrmId));
			Add(&Material.NormalId, sizeof(Material.NormalId));
			Add(&Material.ShadingMode, sizeof(Material.ShadingMode));
			Add(&Material.AlphaCutoff, sizeof(Material.AlphaCutoff));
			Add(&Material.EmissiveId, sizeof(Material.EmissiveId));
		}

		for (u32 i = 0; i < Scene.Meshes.Size(); ++i)
		{
			const auto Handle = Scene.Meshes.GetHandleAtDenseIndex(i);
			const GPUSceneMesh& Mesh = Scene.Meshes.Data()[i];
			Add(&Handle, sizeof(Handle));
			Add(&Mesh.MeshResource, sizeof(Mesh.MeshResource));
			Add(&Mesh.MaterialId, sizeof(Mesh.MaterialId));
		}
		return Signature;
	}

	static void BeginFrame(RenderGraph& Graph, const std::array<RenderGraphBufferRef, 4>& Buffers, bool Clear)
	{
		RenderPassDependencies Dependencies(Graph.Allocator);

		for (RenderGraphBufferRef Buffer : Buffers)
		{
			Dependencies.WriteBuffer(Buffer, VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		}

		Graph.AddPass("SHaRC Begin", RenderGraphPassType::Compute, {}, Dependencies, [Buffers, Clear](RenderGraphContext& Context)
		{
			// Persistent buffers can still be in use by the previous submitted frame.
			VkMemoryBarrier Barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
			Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			vkCmdPipelineBarrier(Context.CommandBuffer->_CmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &Barrier, 0, nullptr, 0, nullptr);

			for (u32 i = Clear ? 0 : 3; i < Buffers.size(); ++i)
			{
				const auto Buffer = Context.GetRenderGraphBuffer(Buffers[i]);
				Context.CommandBuffer->MemsetBuffer(Buffer.get(), 0, Buffer->GetDesc().Size, 0);
			}
		});
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Update and resolve passes

	iVector2 CalculateUpdateGrid(iVector2 RenderSize, u32 PathBudget)
	{
		if (RenderSize.X <= 0 || RenderSize.Y <= 0 || PathBudget == 0)
		{
			return iVector2(0);
		}

		// Preserve sparse updates for small views instead of spending the entire budget.
		const iVector2 MaximumGrid  = (RenderSize - 1) / 8 + 1;
		const u64      MaximumPaths = u64(MaximumGrid.X) * u64(MaximumGrid.Y);

		if (MaximumPaths <= PathBudget)
		{
			return MaximumGrid;
		}

		// Fit a grid with approximately square screen-space cells. Integer division
		// keeps the dispatched path count within budget, including very narrow views.
		const double IdealHeight = std::sqrt(double(PathBudget) * MaximumGrid.Y / MaximumGrid.X);

		const u32 Height = std::clamp(u32(std::round(IdealHeight)), 1u, std::min(u32(MaximumGrid.Y), PathBudget));
		const u32 Width  = std::min(u32(MaximumGrid.X), PathBudget / Height);
		return iVector2(Width, Height);
	}

	struct UpdateShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RadianceCache/RadianceCacheTrace.csd";
		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}
		struct PipelinePermutation {};
		static RayTracingPipelineDesc BuildPipelineDesc(const PipelinePermutation&)
		{
			RayTracingPipelineDesc Desc{};
			Desc.Name = "SHaRC update";
			Desc.MaxRecursionDepth = 1;
			return Desc;
		}

		struct Parameters
		{
			ShaderGPUScene Scene;
			ShaderAccelerationStructure AccelerationStructure;
			ShaderSampledTexture Depth{ TextureBindingFlags::AspectDepth };
			ShaderSampledTexture Normal;
			ShaderSampledTexture Albedo;
			ShaderSampledTexture RoughnessMetallic;
			ShaderReadWriteBuffer HashEntries;
			ShaderReadWriteBuffer Accumulation;
			ShaderReadBuffer Resolved;
			ShaderReadBuffer Statistics;
			ShaderConstants<RadianceCache::Constants> Constants;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.AccelerationStructure, 2, 0);
			Binder.Bind(Params.Depth, 2, 1);
			Binder.Bind(Params.Normal, 2, 2);
			Binder.Bind(Params.Albedo, 2, 3);
			Binder.Bind(Params.RoughnessMetallic, 2, 4);
			Binder.Bind(Params.HashEntries, 3, 0);
			Binder.Bind(Params.Accumulation, 3, 1);
			Binder.Bind(Params.Resolved, 3, 2);
			Binder.Bind(Params.Constants, 3, 3);
			Binder.Bind(Params.Statistics, 3, 4);
		}
	};

	struct ResolveShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RadianceCache/RadianceCacheResolve.csd";
		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderReadWriteBuffer HashEntries;
			ShaderReadWriteBuffer Accumulation;
			ShaderReadWriteBuffer Resolved;
			ShaderReadBuffer Statistics;
			ShaderConstants<RadianceCache::Constants> Constants;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.HashEntries, 0, 0);
			Binder.Bind(Params.Accumulation, 0, 1);
			Binder.Bind(Params.Resolved, 0, 2);
			Binder.Bind(Params.Constants, 0, 3);
			Binder.Bind(Params.Statistics, 0, 4);
		}
	};

	void Prepare(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& Context)
	{
		PROFILE_CPU(CpuCounterRadianceCache);
		State& Cache = Context.RadianceCache;
		RadianceCacheData& Frame = Textures.RadianceCache;
		Frame.DataBuffer = Graph.CreateBuffer(BufferDesc(16, BufferType::UAV), "Baked lighting fallback");
		RenderPassDependencies FallbackDependencies(Graph.Allocator);
		FallbackDependencies.WriteBuffer(Frame.DataBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		Graph.AddPass("Baked lighting fallback", RenderGraphPassType::Compute, {}, FallbackDependencies, [BufferRef = Frame.DataBuffer](RenderGraphContext& Context)
		{
			const auto Buffer = Context.GetRenderGraphBuffer(BufferRef);
			Context.CommandBuffer->MemsetBuffer(Buffer.get(), 0, 16, 0);
		});

		const bool Reflections = IsReflectionCacheRequested(View);
		const bool GI = View.DeferredSettings.RTGIUseRadianceCache && View.DeferredSettings.GlobalIlluminationMode == EDeferredGlobalIlluminationMode::RTGI;
		const bool Visualise = View.DeferredSettings.VisualisationMode == EDeferredRenderVisualisationMode::RadianceCache;
		const bool ValidSize = View.RenderSize.X > 0 && View.RenderSize.Y > 0;
		const bool Enabled = ValidSize && CVar_Enable.GetValue() && Graph.Device->SupportsRadianceCache() && (Reflections || GI || Visualise);

		if (!Enabled)
		{
			Cache.ActiveLastFrame = false;
			return;
		}

		const u32 EntryCount = std::bit_ceil(u32(std::clamp(CVar_Entries.GetValue(), 65536, 4194304)));
		const bool Reallocated = EnsureResources(Graph, Cache, EntryCount);
		AddProfilingCount(CountRadianceCacheAllocations, Reallocated ? 1 : 0);
		const float SceneScale = std::clamp(CVar_SceneScale.GetValue(), 1.0f, 256.0f);
		const u64 Signature = ContentSignature(*Graph.Scene);
		const u32 ResetGeneration = u32(CVar_Reset.GetValue());
		const bool CameraCut = (View.CameraCur.Pos - Cache.PreviousCameraPosition).Length() > 20.0f;
		const bool SceneChanged = Cache.SceneIdentity != Graph.Scene.get() || Cache.ContentSignature != Signature;
		const bool SettingsChanged = Cache.ResetGeneration != ResetGeneration || Cache.SceneScale != SceneScale;
		const bool Clear = Reallocated || !Cache.ActiveLastFrame || SceneChanged || SettingsChanged || CameraCut;
		AddProfilingCount(CountRadianceCacheResets, Clear ? 1 : 0);

		std::array<RenderGraphBufferRef, 4> Buffers;

		for (u32 i = 0; i < Buffers.size(); ++i)
		{
			Buffers[i] = Graph.RegisterExternalBuffer(Cache.Buffers[i], BufferNames[i]);
		}

		const u32      PathBudget = u32(std::clamp(CVar_UpdatePathBudget.GetValue(), 1, 1048576));
		const iVector2 UpdateGrid = CalculateUpdateGrid(View.RenderSize, PathBudget);
		AddProfilingCount(CountRadianceCacheUpdatePaths, u64(UpdateGrid.X) * u64(UpdateGrid.Y));

		Constants Constants{};
		Constants.CameraPositionScale = Vector4(View.CameraCur.Pos, SceneScale);
		Constants.PreviousCameraPosition = Vector4(Clear ? View.CameraCur.Pos : Cache.PreviousCameraPosition, 0);
		Constants.Config = iVector4(EntryCount, Cache.FrameIndex++, std::clamp(CVar_AccumulationFrames.GetValue(), 1, 64), std::clamp(CVar_StaleFrames.GetValue(), 8, 128));
		Constants.Update = iVector4(UpdateGrid.X, UpdateGrid.Y, 0, std::clamp(CVar_Bounces.GetValue(), 1, 8));
		Constants.Flags = iVector4(CVar_Statistics.GetValue(), Reflections, GI, std::clamp(CVar_Visualisation.GetValue(), 0, 2));

		Frame.Query.Enabled = true;
		Frame.Query.HashEntries = Buffers[0];
		Frame.Query.Accumulation = Buffers[1];
		Frame.Query.Resolved = Buffers[2];
		Frame.Query.Statistics = Buffers[3];
		Frame.Query.Constants.Value = Constants;
		BeginFrame(Graph, Buffers, Clear);

		UpdateShader::Parameters Update;
		Update.Scene.UseCombinedSampler = false;
		Update.Depth = Textures.GBufferDS;
		Update.Normal = Textures.GBufferNormal;
		Update.Albedo = Textures.GBufferAlbedo;
		Update.RoughnessMetallic = Textures.GBufferRM;
		Update.HashEntries = Buffers[0];
		Update.Accumulation = Buffers[1];
		Update.Resolved = Buffers[2];
		Update.Statistics = Buffers[3];
		Update.Constants.Value = Constants;
		RenderPassDependencies UpdateDependencies(Graph.Allocator);
		UpdateDependencies.Bind<UpdateShader>(Update);

		Graph.AddPass("SHaRC Update", RenderGraphPassType::Compute, {}, UpdateDependencies, [Update, UpdateGrid](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRadianceCacheUpdate, Context);
			auto Params = Update;
			Params.AccelerationStructure = Context.Scene->TLAS;
			auto* Pipeline = GetRayTracingPipeline<UpdateShader>(Context, {}, {});
			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.BindRayTracingParameters<UpdateShader>(Pipeline, Params);
			Context.CommandBuffer->TraceRays(Pipeline, UpdateGrid.X, UpdateGrid.Y, 1);
		});

		ResolveShader::Parameters Resolve;
		Resolve.HashEntries = Buffers[0];
		Resolve.Accumulation = Buffers[1];
		Resolve.Resolved = Buffers[2];
		Resolve.Statistics = Buffers[3];
		Resolve.Constants.Value = Constants;
		RenderPassDependencies ResolveDependencies(Graph.Allocator);
		ResolveDependencies.Bind<ResolveShader>(Resolve);

		Graph.AddPass("SHaRC Resolve", RenderGraphPassType::Compute, {}, ResolveDependencies, [Resolve, EntryCount](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterRadianceCacheResolve, Context);
			auto* Pipeline = GetComputePipeline<ResolveShader>(Context, {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<ResolveShader>(Pipeline, Resolve);
			Context.CommandBuffer->Dispatch((EntryCount + 255) / 256, 1, 1);
		});

		Cache.SceneIdentity = Graph.Scene.get();
		Cache.ContentSignature = Signature;
		Cache.ResetGeneration = ResetGeneration;
		Cache.SceneScale = SceneScale;
		Cache.PreviousCameraPosition = View.CameraCur.Pos;
		Cache.ActiveLastFrame = true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Cache visualisation

	struct VisualiseShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RadianceCache/RadianceCacheVisualise.csd";
		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}
		struct PipelinePermutation {};
		static GraphicsPipelineDesc BuildPipelineDesc(const PipelinePermutation&)
		{
			GraphicsPipelineDesc Desc;
			Desc.Name = "SHaRC visualisation";
			Desc.rasterizerState.Cull = CullMode::No;
			Desc.blendState.RenderTargets = { RenderTargetBlendDesc() };
			Desc.depthStencilState.DepthEnable = false;
			Desc.depthStencilState.DepthWriteMask = false;
			return Desc;
		}

		struct Parameters
		{
			ShaderReadBuffer Scene;
			ShaderSampledTexture Depth{ TextureBindingFlags::AspectDepth };
			ShaderSampledTexture Normal;
			QueryParameters Cache;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene, 0, 0);
			Binder.Bind(Params.Depth, 0, 1);
			Binder.Bind(Params.Normal, 0, 2);
			Params.Cache.Bind(Binder, 1);
		}
	};

	RenderGraphTextureRef VisualiseRadianceCache(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures)
	{
		TextureDesc2 Desc;
		Desc.Format = TextureFormat::R11G11B10F;
		Desc.Usage = TextureUsage::RenderTargetColor;
		Desc.Width = View.RenderSize.X;
		Desc.Height = View.RenderSize.Y;
		const auto Target = Graph.CreateTexture(Desc, "SHaRC Visualisation");
		RenderPassParameters Pass;
		Pass.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Target, {} };
		Pass.ViewportSize = View.RenderSize;

		if (!Textures.RadianceCache.Query.Enabled)
		{
			Log::Error("RadianceCache visualisation requires SHaRC enabled and a supported GPU");
			Graph.AddPass("SHaRC Unavailable", RenderGraphPassType::Raster, Pass, RenderPassDependencies(Graph.Allocator), [](RenderGraphContext&) {});
			return Target;
		}

		VisualiseShader::Parameters Params;
		Params.Depth = Textures.GBufferDS;
		Params.Normal = Textures.GBufferNormal;
		Params.Cache = Textures.RadianceCache.Query;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<VisualiseShader>(Params);

		Graph.AddPass("SHaRC Visualisation", RenderGraphPassType::Raster, Pass, Dependencies, [Params](RenderGraphContext& Context)
		{
			auto Parameters = Params;
			Parameters.Scene = Context.Scene->SceneBuffer;
			auto* Pipeline = GetGraphicsPipeline<VisualiseShader>(Context, {}, {});
			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.BindGraphicsParameters<VisualiseShader>(Pipeline, Parameters);
			Context.CommandBuffer->Draw(3, 1, 0, 0);
		});

		return Target;
	}
}
