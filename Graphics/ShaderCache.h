#pragma once

#include <Graphics/RenderGraph.h>
#include <Graphics/ShaderBinder.h>
#include <Graphics/ShaderHotReload.h>

#include <string>
#include <unordered_map>

namespace Columbus
{
	struct ShaderCacheKey
	{
		uint32_t ShaderPermutationIndex = 0;
		size_t PipelinePermutationHash = 0;
		size_t RenderPassHash = 0;

		bool operator==(const ShaderCacheKey& Other) const
		{
			return ShaderPermutationIndex == Other.ShaderPermutationIndex &&
				PipelinePermutationHash == Other.PipelinePermutationHash &&
				RenderPassHash == Other.RenderPassHash;
		}
	};

	struct ShaderCacheKeyHash
	{
		size_t operator()(const ShaderCacheKey& Key) const
		{
			size_t Hash = Key.PipelinePermutationHash;
			Hash ^= (size_t)Key.ShaderPermutationIndex + 0x9e3779b9 + (Hash << 6) + (Hash >> 2);
			Hash ^= Key.RenderPassHash + 0x9e3779b9 + (Hash << 6) + (Hash >> 2);
			return Hash;
		}
	};

	struct ShaderCacheEntry
	{
		CompiledShaderData PackedBytecode;
		bool bLoadedBytecode = false;
		std::unordered_map<ShaderCacheKey, ComputePipeline*, ShaderCacheKeyHash> ComputePipelines;
		std::unordered_map<ShaderCacheKey, GraphicsPipeline*, ShaderCacheKeyHash> GraphicsPipelines;
		std::unordered_map<ShaderCacheKey, RayTracingPipeline*, ShaderCacheKeyHash> RayTracingPipelines;
	};

	class ShaderCache
	{
	public:
		template <typename TShader>
		ComputePipeline* GetComputePipeline(
			RenderGraphContext& Context,
			ShaderHotReload& HotReload,
			const typename TShader::Permutation& ShaderPermutation)
		{
			ShaderCacheEntry& Entry = GetOrLoadEntry<TShader>(HotReload);
			const uint32_t ShaderPermutationIndex = GetShaderPermutationIndex<TShader>(Entry.PackedBytecode, ShaderPermutation);
			const ShaderCacheKey CacheKey {
				.ShaderPermutationIndex = ShaderPermutationIndex,
				.PipelinePermutationHash = 0,
				.RenderPassHash = 0,
			};

			auto It = Entry.ComputePipelines.find(CacheKey);
			if (It != Entry.ComputePipelines.end())
				return It->second;

			const CompiledShaderPermutation& Bytecode = Entry.PackedBytecode.Permutations[ShaderPermutationIndex];

			ComputePipelineDesc Desc;
			Desc.Name = Entry.PackedBytecode.Name + "_" + Bytecode.Name;
			Desc.Shader = Bytecode;

			ComputePipeline* Pipeline = Context.Device->CreateComputePipeline(Desc);
			Entry.ComputePipelines[CacheKey] = Pipeline;
			return Pipeline;
		}

		template <typename TShader>
		GraphicsPipeline* GetGraphicsPipeline(
			RenderGraphContext& Context,
			ShaderHotReload& HotReload,
			const typename TShader::Permutation& ShaderPermutation,
			const typename TShader::PipelinePermutation& PipelinePermutation)
		{
			ShaderCacheEntry& Entry = GetOrLoadEntry<TShader>(HotReload);
			const uint32_t ShaderPermutationIndex = GetShaderPermutationIndex<TShader>(Entry.PackedBytecode, ShaderPermutation);
			const ShaderCacheKey CacheKey {
				.ShaderPermutationIndex = ShaderPermutationIndex,
				.PipelinePermutationHash = HashPipelinePermutation(PipelinePermutation),
				.RenderPassHash = (size_t)Context.VulkanRenderPass,
			};

			auto It = Entry.GraphicsPipelines.find(CacheKey);
			if (It != Entry.GraphicsPipelines.end())
				return It->second;

			const CompiledShaderPermutation& Bytecode = Entry.PackedBytecode.Permutations[ShaderPermutationIndex];

			GraphicsPipelineDesc Desc = TShader::BuildPipelineDesc(PipelinePermutation);
			Desc.Name += "_" + Bytecode.Name;
			Desc.Shader = Bytecode;

			GraphicsPipeline* Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			Entry.GraphicsPipelines[CacheKey] = Pipeline;
			return Pipeline;
		}

		template <typename TShader>
		RayTracingPipeline* GetRayTracingPipeline(
			RenderGraphContext& Context,
			ShaderHotReload& HotReload,
			const typename TShader::Permutation& ShaderPermutation,
			const typename TShader::PipelinePermutation& PipelinePermutation)
		{
			ShaderCacheEntry& Entry = GetOrLoadEntry<TShader>(HotReload);
			const uint32_t ShaderPermutationIndex = GetShaderPermutationIndex<TShader>(Entry.PackedBytecode, ShaderPermutation);
			const ShaderCacheKey CacheKey {
				.ShaderPermutationIndex = ShaderPermutationIndex,
				.PipelinePermutationHash = HashPipelinePermutation(PipelinePermutation),
				.RenderPassHash = 0,
			};

			auto It = Entry.RayTracingPipelines.find(CacheKey);
			if (It != Entry.RayTracingPipelines.end())
				return It->second;

			const CompiledShaderPermutation& Bytecode = Entry.PackedBytecode.Permutations[ShaderPermutationIndex];

			RayTracingPipelineDesc Desc = TShader::BuildPipelineDesc(PipelinePermutation);
			Desc.Name += "_" + Bytecode.Name;
			Desc.Shader = Bytecode;

			RayTracingPipeline* Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
			Entry.RayTracingPipelines[CacheKey] = Pipeline;
			return Pipeline;
		}

		void Clear()
		{
			Entries.clear();
		}

		void InvalidateAll()
		{
			Clear();
		}

	private:
		template <typename TShader>
		ShaderCacheEntry& GetOrLoadEntry(ShaderHotReload& HotReload)
		{
			ShaderCacheEntry& Entry = Entries[TShader::Path];
			if (HotReload.ReloadShaderDataIfNeeded(TShader::Path, Entry.PackedBytecode, Entry.bLoadedBytecode))
			{
				Entry.ComputePipelines.clear();
				Entry.GraphicsPipelines.clear();
				Entry.RayTracingPipelines.clear();
				Log::Message("[ShaderHotReload] Cleared cached pipelines for %s", TShader::Path);
			}

			return Entry;
		}

		std::unordered_map<std::string, ShaderCacheEntry> Entries;
	};


	// =================================================================================================================
	// Pass-facing shader helpers below.
	// =================================================================================================================

	template <typename TShader, typename TParameters>
	void RenderGraphContext::BindComputePipeline(const ComputePipeline* Pipeline, const TParameters& Parameters)
	{
		CommandBuffer->BindComputePipeline(Pipeline);
		BindComputeParameters<TShader>(Pipeline, Parameters);
	}

	template <typename TShader, typename TParameters>
	void RenderGraphContext::BindGraphicsPipeline(const GraphicsPipeline* Pipeline, const TParameters& Parameters)
	{
		CommandBuffer->BindGraphicsPipeline(Pipeline);
		BindGraphicsParameters<TShader>(Pipeline, Parameters);
	}

	template <typename TShader, typename TParameters>
	void RenderGraphContext::BindComputeParameters(const ComputePipeline* Pipeline, const TParameters& Parameters)
	{
		ShaderBinderBackendStateVulkan Backend;
		ShaderBinder Binder(*this, Pipeline, Backend);
		TShader::Bind(Binder, Parameters);
		Binder.Flush();
	}

	template <typename TShader, typename TParameters>
	void RenderGraphContext::BindGraphicsParameters(const GraphicsPipeline* Pipeline, const TParameters& Parameters)
	{
		ShaderBinderBackendStateVulkan Backend;
		ShaderBinder Binder(*this, Pipeline, Backend);
		TShader::Bind(Binder, Parameters);
		Binder.Flush();
	}

	template <typename TShader, typename TParameters>
	void RenderGraphContext::BindRayTracingParameters(const RayTracingPipeline* Pipeline, const TParameters& Parameters)
	{
		ShaderBinderBackendStateVulkan Backend;
		ShaderBinder Binder(*this, Pipeline, Backend);
		TShader::Bind(Binder, Parameters);
		Binder.Flush();
	}

	template <typename TShader, typename TParameters>
	void RenderGraphContext::BindRayTracingPipeline(const RayTracingPipeline* Pipeline, const TParameters& Parameters)
	{
		CommandBuffer->BindRayTracingPipeline(Pipeline);
		BindRayTracingParameters<TShader>(Pipeline, Parameters);
	}

	template <typename TShader>
	GraphicsPipeline* GetGraphicsPipeline(
		RenderGraphContext& Context,
		const typename TShader::Permutation& ShaderPermutation,
		const typename TShader::PipelinePermutation& PipelinePermutation)
	{
		Context.Device->ShaderHotReload->Update(*Context.Device->ShaderCache);
		return Context.Device->ShaderCache->GetGraphicsPipeline<TShader>(Context, *Context.Device->ShaderHotReload, ShaderPermutation, PipelinePermutation);
	}

	template <typename TShader>
	ComputePipeline* GetComputePipeline(RenderGraphContext& Context, const typename TShader::Permutation& ShaderPermutation)
	{
		Context.Device->ShaderHotReload->Update(*Context.Device->ShaderCache);
		return Context.Device->ShaderCache->GetComputePipeline<TShader>(Context, *Context.Device->ShaderHotReload, ShaderPermutation);
	}

	template <typename TShader>
	RayTracingPipeline* GetRayTracingPipeline(
		RenderGraphContext& Context,
		const typename TShader::Permutation& ShaderPermutation,
		const typename TShader::PipelinePermutation& PipelinePermutation)
	{
		Context.Device->ShaderHotReload->Update(*Context.Device->ShaderCache);
		return Context.Device->ShaderCache->GetRayTracingPipeline<TShader>(Context, *Context.Device->ShaderHotReload, ShaderPermutation, PipelinePermutation);
	}
}
