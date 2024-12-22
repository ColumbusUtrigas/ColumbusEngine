#include "RenderGraph.h"
#include "Common/Image/Image.h"
#include "Core/Util.h"
#include "Graphics/Core/Buffer.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Core/Types.h"
#include "Graphics/Vulkan/BufferVulkan.h"
#include "Graphics/Vulkan/CommandBufferVulkan.h"
#include "Graphics/Vulkan/SwapchainVulkan.h"
#include "Graphics/Vulkan/PipelinesVulkan.h"
#include "Graphics/Vulkan/TextureVulkan.h"
#include "Graphics/Vulkan/TypeConversions.h"
#include "Core/Hash.h"
#include "Math/Vector2.h"
#include "Profiling/Profiling.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <vulkan/vulkan.hpp>

IMPLEMENT_CPU_PROFILING_COUNTER("RG Clear", "RenderGraph", Counter_RenderGraphClear);
IMPLEMENT_CPU_PROFILING_COUNTER("RG Build", "RenderGraph", Counter_RenderGraphBuild);
IMPLEMENT_CPU_PROFILING_COUNTER("RG Execute", "RenderGraph", Counter_RenderGraphExecute);

IMPLEMENT_MEMORY_PROFILING_COUNTER("RG Textures", "RenderGraphMemory", MemoryCounter_RenderGraphTextures);
IMPLEMENT_MEMORY_PROFILING_COUNTER("RG Buffers", "RenderGraphMemory", MemoryCounter_RenderGraphBuffers);
IMPLEMENT_MEMORY_PROFILING_COUNTER("RG CPU Side", "RenderGraphMemory", MemoryCounter_RenderGraphCPU);
IMPLEMENT_MEMORY_PROFILING_COUNTER("RG Allocator Waste", "RenderGraphMemory", MemoryCounter_RenderGraphAllocatorWaste);

IMPLEMENT_GPU_PROFILING_COUNTER("RG GPU Frame", "RenderGraphGPU", GpuCounter_RenderGraphFrame);

namespace std {
	template <> struct hash<Columbus::RenderPassAttachment>
	{
		size_t operator()(const Columbus::RenderPassAttachment & x) const
		{
			size_t Hash = 0;
			hash_combine(Hash, x.LoadOp);
			hash_combine(Hash, x.Texture);
			return Hash;
		}
	};
}

size_t Columbus::HashRenderPassParameters::operator()(const Columbus::RenderPassParameters& Params) const
{
	size_t Hash = 0;

	for (auto& A : Params.ColorAttachments)
		hash_combine(Hash, A);

	hash_combine(Hash, Params.DepthStencilAttachment);

	return Hash;
}

namespace Columbus
{

	void RenderGraphContext::DispatchComputePixels(const ComputePipeline* Pipeline, iVector3 GroupSize, iVector3 Pixels)
	{
		const iVector3 Groups = (Pixels + GroupSize - 1) / GroupSize;
		CommandBuffer->Dispatch(Groups.X, Groups.Y, Groups.Z);
	}

	void RenderGraphData::ClearDescriptorData()
	{
		for (auto& DescriptorData : DescriptorSets[CurrentPerFrameData])
		{
			memset(DescriptorData.second.NumUsedThisFrame, 0, sizeof(DescriptorData.second.NumUsedThisFrame));
		}
	}

	template <typename PipelineType, typename VulkanPipelineType>
	static VkDescriptorSet GetDescriptorSetCached(RenderGraphData& RenderData, SPtr<DeviceVulkan> Device, const PipelineType* InPipeline, int Index)
	{
		auto vkpipe = static_cast<const VulkanPipelineType*>(InPipeline);
		VkPipeline Pipeline = vkpipe->pipeline;

		auto& DescriptorCache = RenderData.DescriptorSets[RenderData.CurrentPerFrameData];
		if (DescriptorCache.find(Pipeline) == DescriptorCache.end())
		{
			auto DescriptorSetData = RenderGraphData::PipelineDescriptorSetFrameData();

			DescriptorCache[Pipeline] = DescriptorSetData;
		}

		auto& PipelineDescriptorsData = DescriptorCache[Pipeline];

		if (PipelineDescriptorsData.NumUsedThisFrame[Index] >= PipelineDescriptorsData.DescriptorSets[Index].size())
		{
			PipelineDescriptorsData.DescriptorSets[Index].push_back(Device->CreateDescriptorSet(InPipeline, Index));
		}

		return PipelineDescriptorsData.DescriptorSets[Index][PipelineDescriptorsData.NumUsedThisFrame[Index]++];
	}

	VkDescriptorSet RenderGraphContext::GetDescriptorSet(const ComputePipeline* Pipeline, int Index)
	{
		return GetDescriptorSetCached<ComputePipeline, ComputePipelineVulkan>(RenderData, Device, Pipeline, Index);
	}

	VkDescriptorSet RenderGraphContext::GetDescriptorSet(const GraphicsPipeline* Pipeline, int Index)
	{
		return GetDescriptorSetCached<GraphicsPipeline, GraphicsPipelineVulkan>(RenderData, Device, Pipeline, Index);
	}

	VkDescriptorSet RenderGraphContext::GetDescriptorSet(const RayTracingPipeline* Pipeline, int Index)
	{
		return GetDescriptorSetCached<RayTracingPipeline, RayTracingPipelineVulkan>(RenderData, Device, Pipeline, Index);
	}

	void RenderGraphContext::BindGPUScene(const GraphicsPipeline* Pipeline, bool UseCombinedSampler)
	{
		CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, UseCombinedSampler ? &RenderData.GPUSceneData.TextureSet : &RenderData.GPUSceneData.TextureSetNonCombined);
		CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 1, 1, &RenderData.GPUSceneData.SceneSet);
	}

	void RenderGraphContext::BindGPUScene(const RayTracingPipeline* Pipeline, bool UseCombinedSampler)
	{
		CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 0, 1, UseCombinedSampler ? &RenderData.GPUSceneData.TextureSet : &RenderData.GPUSceneData.TextureSetNonCombined);
		CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 1, 1, &RenderData.GPUSceneData.SceneSet);
	}

	SPtr<Texture2> RenderGraphContext::GetRenderGraphTexture(RenderGraphTextureRef Ref)
	{
		bool ValidUsage = false;

		// TODO: optimize or give option to turn off validation
		for (const auto& Read : Graph.Passes[CurrentPass].Dependencies.TextureReadResources)
		{
			if (Read.Texture == Ref)
			{
				ValidUsage = true;
				break;
			}
		}

		for (const auto& Write : Graph.Passes[CurrentPass].Dependencies.TextureWriteResources)
		{
			if (Write.Texture == Ref)
			{
				ValidUsage = true;
				break;
			}
		}

		if (!ValidUsage)
		{
			Log::Error("RenderGraph Validation: Texture %s isn't defined as a dependency of pass %s, but this pass tries to access it",
				Graph.Textures[Ref].DebugName.c_str(), Graph.Passes[CurrentPass].Name);
		}

		return Graph.Textures[Ref].Texture;
	}

	SPtr<Buffer> RenderGraphContext::GetRenderGraphBuffer(RenderGraphBufferRef Ref)
	{
		bool ValidUsage = false;

		// TODO: optimize or give option to turn off validation
		for (const auto& Read : Graph.Passes[CurrentPass].Dependencies.BufferReadResources)
		{
			if (Read.Buffer == Ref)
			{
				ValidUsage = true;
				break;
			}
		}

		for (const auto& Write : Graph.Passes[CurrentPass].Dependencies.BufferWriteResources)
		{
			if (Write.Buffer == Ref)
			{
				ValidUsage = true;
				break;
			}
		}

		if (!ValidUsage)
		{
			Log::Error("RenderGraph Validation: Buffer %s isn't defined as a dependency of pass %s, but this pass tries to access it",
				Graph.Buffers[Ref].DebugName.c_str(), Graph.Passes[CurrentPass].Name);
		}

		return Graph.Buffers[Ref].Buffer;
	}

	RenderGraphScopedMarker::RenderGraphScopedMarker(RenderGraph& Graph, const char* Marker) : Graph(Graph)
	{
		Graph.PushMarker(Marker);
	}

	RenderGraphScopedMarker::~RenderGraphScopedMarker()
	{
		Graph.PopMarker();
	}

	RenderGraph::RenderGraph(SPtr<DeviceVulkan> Device, SPtr<GPUScene> Scene) : Device(Device), Scene(Scene)
	{
		for (auto& PerFrameData : RenderData.PerFrameData)
		{
			PerFrameData.SubmitSemaphore = Device->CreateSemaphore();
		}

		// Assuming that GPUScene is completely static, fill it in
		// TODO: fully refactor
		// TODO: move to GPUScene

		if (Scene)
		{
			// brute-generate VkDescriptorSetLayout
			auto CreateLayout = [&](uint32_t bindingNum, std::span<VkDescriptorType> types, uint32_t count, bool unbounded = true, u32 bindingCount = 1)
			{
				VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

				VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo;
				bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
				bindingFlagsInfo.pNext = nullptr;
				bindingFlagsInfo.bindingCount = 1;
				bindingFlagsInfo.pBindingFlags = &bindingFlags;

				fixed_vector<VkDescriptorSetLayoutBinding, 16> bindings;
				for (u32 i = 0; i < bindingCount; i++)
				{
					VkDescriptorSetLayoutBinding binding;
					binding.binding = bindingNum + i;
					binding.descriptorType = types.size() > i ? types[i] : types[0];
					binding.descriptorCount = count;
					binding.stageFlags = VK_SHADER_STAGE_ALL;
					binding.pImmutableSamplers = nullptr;
					bindings.push_back(binding);
				}

				VkDescriptorSetLayoutCreateInfo setLayoutInfo;
				setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				setLayoutInfo.pNext = unbounded ? &bindingFlagsInfo : nullptr;
				setLayoutInfo.flags = 0;
				setLayoutInfo.bindingCount = bindingCount;
				setLayoutInfo.pBindings = bindings.data();

				VkDescriptorSetLayout layout;

				VK_CHECK(vkCreateDescriptorSetLayout(Device->_Device, &setLayoutInfo, nullptr, &layout));

				return layout;
			};

			// TODO: unify texture set
			VkDescriptorType TextureTypes[] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
			VkDescriptorType TextureTypesNonCombined[] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE };
			VkDescriptorType SceneTypes[] = {
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_DESCRIPTOR_TYPE_SAMPLER
			};

			RenderData.GPUSceneLayout.TextureLayout = CreateLayout(0, TextureTypes, 2000);
			RenderData.GPUSceneLayout.SceneLayout = CreateLayout(0, SceneTypes, 1, false, 5);

			auto TextureLayoutNonCombined = CreateLayout(0, TextureTypesNonCombined, 2000);

			RenderData.GPUSceneData.TextureSet = Device->CreateDescriptorSetUnbounded(RenderData.GPUSceneLayout.TextureLayout, 2000);
			RenderData.GPUSceneData.TextureSetNonCombined = Device->CreateDescriptorSetUnbounded(TextureLayoutNonCombined, 2000);
			RenderData.GPUSceneData.SceneSet = Device->CreateDescriptorSetUnbounded(RenderData.GPUSceneLayout.SceneLayout, 0);


			Device->UpdateDescriptorSet(RenderData.GPUSceneData.SceneSet, 0, 0, Scene->SceneBuffer);
			Device->UpdateDescriptorSet(RenderData.GPUSceneData.SceneSet, 1, 0, Scene->LightsBuffer);
			Device->UpdateDescriptorSet(RenderData.GPUSceneData.SceneSet, 2, 0, Scene->MeshesBuffer);
			Device->UpdateDescriptorSet(RenderData.GPUSceneData.SceneSet, 3, 0, Scene->MaterialsBuffer);
			Device->UpdateDescriptorSet(RenderData.GPUSceneData.SceneSet, 4, 0, Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::Repeat>());
		}
	}

	RenderGraph::~RenderGraph()
	{
		// TODO: move to GPUScene
		//vkFreeDescriptorSets(Device->_Device, Device->_DescriptorPool, 1, &RenderData.GPUSceneData.TextureSet);
		//vkFreeDescriptorSets(Device->_Device, Device->_DescriptorPool, 1, &RenderData.GPUSceneData.SceneSet);
		vkDestroyDescriptorSetLayout(Device->_Device, RenderData.GPUSceneLayout.TextureLayout, nullptr);
		vkDestroyDescriptorSetLayout(Device->_Device, RenderData.GPUSceneLayout.SceneLayout, nullptr);

		for (int i = 0; i < MaxFramesInFlight; i++)
		{
			auto& Data = RenderData.PerFrameData[i];

			vkDestroySemaphore(Device->_Device, Data.SubmitSemaphore, nullptr);

			for (VkSemaphore& Semaph : Data.CommandBuffersSemaphores)
			{
				vkDestroySemaphore(Device->_Device, Semaph, nullptr);
			}

			for (CommandBufferVulkan* CmdBuf : Data.CommandBuffers)
			{
				delete CmdBuf;
			}
		}
	}

	RenderGraphTextureRef RenderGraph::CreateTexture(const TextureDesc2& Desc, const char* Name)
	{
		// on texture creation stage, we just create a description and a handle for it
		// the actual texture will be allocated on graph build stage

		int Id = static_cast<int>(Textures.size());
		Textures.push_back(RenderGraphTexture(Desc, Name, Id, Allocator));
		return Id;
	}

	RenderGraphBufferRef RenderGraph::CreateBuffer(const BufferDesc& Desc, const char* Name)
	{
		int Id = static_cast<int>(Buffers.size());
		Buffers.push_back(RenderGraphBuffer(Desc, Name, Id, Allocator));
		return Id;
	}

	RenderGraphTextureRef RenderGraph::RegisterExternalTexture(SPtr<Texture2> Tex, const char* Name)
	{
		int Id = static_cast<int>(Textures.size());
		auto RGTex = RenderGraphTexture(Tex->GetDesc(), Name, Id, Allocator);
		RGTex.Texture = Tex;
		RGTex.bExternal = true;
		Textures.push_back(RGTex);
		return Id;
	}

	TextureDesc2 RenderGraph::GetTextureDesc(RenderGraphTextureRef Texture) const
	{
		return Textures[Texture].Desc;
	}

	BufferDesc RenderGraph::GetBufferDesc(RenderGraphBufferRef Buffer) const
	{
		return Buffers[Buffer].Desc;
	}

	const char* RenderGraph::GetTextureName(RenderGraphTextureRef Texture) const
	{
		return Textures[Texture].DebugName.c_str();
	}

	const char* RenderGraph::GetBufferName(RenderGraphBufferRef Buffer) const
	{
		return Buffers[Buffer].DebugName.c_str();
	}

	iVector2 RenderGraph::GetTextureSize2D(RenderGraphTextureRef Texture) const
	{
		return iVector2((int)Textures[Texture].Desc.Width, (int)Textures[Texture].Desc.Height);
	}

	Texture2* RenderGraph::GetTextureAfterExecution(RenderGraphTextureRef Texture) const
	{
		COLUMBUS_ASSERT(ExecutionHasFinished && "Trying to read RG texture before execution");
		return Textures[Texture].Texture.get();
	}

	SPtr<Texture2> RenderGraph::GetTextureAfterExecutionShared(RenderGraphTextureRef Texture) const
	{
		COLUMBUS_ASSERT(ExecutionHasFinished && "Trying to read RG texture before execution");
		return Textures[Texture].Texture;
	}

	// TODO: unify allocation logic for textures and buffers?
	void RenderGraph::AllocateTexture(RenderGraphTexture& Texture)
	{
		if (Texture.bExternal)
			return; // don't do anything with the external resource

		const auto ApplyTextureFromPool = [&Texture, this](RenderGraphPooledTexture& PooledTexture) {
			Texture.Texture = PooledTexture.Texture;
			Texture.AllocatedSize = PooledTexture.Texture->GetSize();
			PooledTexture.Used = true;
			PooledTexture.UnusedFrames = 0;
			Device->SetDebugName(Texture.Texture.get(), Texture.DebugName.c_str());
		};

		if (TextureResourcePool.contains(Texture.Desc))
		{
			auto& TextureResources = TextureResourcePool[Texture.Desc];

			// Search for the first unused texture with matching Desc
			auto FoundRef = std::find_if(TextureResources.begin(), TextureResources.end(), [](RenderGraphPooledTexture& Tex) {
				return !Tex.Used;
			});

			if (FoundRef != TextureResources.end()) // Found
			{
				ApplyTextureFromPool(*FoundRef);
				return;
			}
		} else
		{
			// No matching Desc, ensure that list exists
			TextureResourcePool[Texture.Desc] = std::vector<RenderGraphPooledTexture>();
		}

		Log::Message("RenderGraph texture allocated: %s (%ix%ix%i)", Texture.DebugName.c_str(), Texture.Desc.Width, Texture.Desc.Height, Texture.Desc.Depth);

		// No available texture with matching Desc found, create a new one
		RenderGraphPooledTexture& Result = TextureResourcePool[Texture.Desc].emplace_back(
			RenderGraphPooledTexture{ SPtr<Texture2>(Device->CreateTexture(Texture.Desc)), false }
		);
		ApplyTextureFromPool(Result);
		AddProfilingMemory(MemoryCounter_RenderGraphTextures, Result.Texture->GetSize());
	}

	// TODO: unify allocation logic for textures and buffers?
	void RenderGraph::AllocateBuffer(RenderGraphBuffer& Buffer)
	{
		const auto ApplyBufferFromPool = [&Buffer, this](RenderGraphPooledBuffer& PooledBuffer) {
			Buffer.Buffer = PooledBuffer.Buffer;
			Buffer.AllocatedSize = PooledBuffer.Buffer->GetSize();
			PooledBuffer.Used = true;
			PooledBuffer.UnusedFrames = 0;
			Device->SetDebugName(Buffer.Buffer.get(), Buffer.DebugName.c_str());
		};

		if (BufferResourcePool.contains(Buffer.Desc))
		{
			auto& BufferResources = BufferResourcePool[Buffer.Desc];

			// Search for the first unused buffer with matching Desc
			auto FoundRef = std::find_if(BufferResources.begin(), BufferResources.end(), [](RenderGraphPooledBuffer& Buf) {
				return !Buf.Used;
			});

			if (FoundRef != BufferResources.end()) // Found
			{
				ApplyBufferFromPool(*FoundRef);
				return;
			}
		} else
		{
			// No matching Desc, ensure that list exists
			BufferResourcePool[Buffer.Desc] = std::vector<RenderGraphPooledBuffer>();
		}

		double Size = 0;
		const char* HumanizedSize = HumanizeBytes(Buffer.Desc.Size, Size);
		Log::Message("RenderGraph buffer allocated: %s (%.2f %s)", Buffer.DebugName.c_str(), Size, HumanizedSize);

		// No available buffer with matching Desc found, create a new one
		RenderGraphPooledBuffer& Result = BufferResourcePool[Buffer.Desc].emplace_back(
			RenderGraphPooledBuffer{ SPtr<Columbus::Buffer>(Device->CreateBuffer(Buffer.Desc, nullptr)), false }
		);
		ApplyBufferFromPool(Result);
		AddProfilingMemory(MemoryCounter_RenderGraphBuffers, Result.Buffer->GetSize());
	}

	void RenderGraph::AddPass(const char* Name, RenderGraphPassType Type, RenderPassParameters Parameters, RenderPassDependencies Dependencies, RenderGraphExecutionFunc ExecuteCallback)
	{
		int PassId = static_cast<int>(Passes.size());
		RenderPass Pass {
			Allocator.StrDup(Name), PassId, Type, Parameters, Dependencies, ExecuteCallback,
			RenderGraphDynamicArray<RenderGraphTextureBarrier>(Allocator),
			RenderGraphDynamicArray<RenderGraphBufferBarrier>(Allocator),
		};

		// treat pass attachments as write dependencies
		for (const auto& Attachment : Parameters.ColorAttachments)
		{
			if (Attachment)
			{
				Pass.Dependencies.Write(Attachment->Texture, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			}
		}

		if (Parameters.DepthStencilAttachment)
		{
			Pass.Dependencies.Write(Parameters.DepthStencilAttachment->Texture, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
		}

		// TODO: validate for duplicates

		// versioning
		for (RenderPassTextureDependency& Read : Pass.Dependencies.TextureReadResources)
		{
			Read.Version = Textures[Read.Texture].Version;
		}

		for (RenderPassBufferDependency& Read : Pass.Dependencies.BufferReadResources)
		{
			Read.Version = Buffers[Read.Buffer].Version;
		}
		
		for (RenderPassTextureDependency& Write : Pass.Dependencies.TextureWriteResources)
		{
			// If there was a version of a texture before, we overwrite this texture and, therefore, depend on it
			if (Textures[Write.Texture].Version >= 0)
			{
				Pass.Dependencies.Read(Write.Texture, Write.Access, Write.Stage);
				Pass.Dependencies.TextureReadResources[Pass.Dependencies.TextureReadResources.size() - 1].Version = Textures[Write.Texture].Version;
			}

			Textures[Write.Texture].Version++;
			Write.Version = Textures[Write.Texture].Version;
		}

		for (RenderPassBufferDependency& Write : Pass.Dependencies.BufferWriteResources)
		{
			// If there was a version of a buffer before, we overwrite this buffer and, therefore, depend on it
			if (Buffers[Write.Buffer].Version >= 0)
			{
				Pass.Dependencies.ReadBuffer(Write.Buffer, Write.Access, Write.Stage);
				Pass.Dependencies.BufferReadResources[Pass.Dependencies.BufferReadResources.size() - 1].Version = Buffers[Write.Buffer].Version;
			}

			Buffers[Write.Buffer].Version++;
			Write.Version = Buffers[Write.Buffer].Version;
		}

		Passes.emplace_back(std::move(Pass));
	}

	void RenderGraph::PushMarker(const char* Marker)
	{
		MarkersStack.push(RenderGraphMarker{
			.Marker = Allocator.StrDup(Marker), .PassId = (RenderGraphPassId)Passes.size(), .Type = RenderGraphMarker::TypePush
		});
	}

	void RenderGraph::PopMarker()
	{
		MarkersStack.push(RenderGraphMarker{
			.Marker = "", .PassId = (RenderGraphPassId)Passes.size(), .Type = RenderGraphMarker::TypePop
		});
	}

	void RenderGraph::ExtractTexture(RenderGraphTextureRef Src, Texture2** Dst)
	{
		Extractions.push_back(RenderGraphTextureExtraction{ .Src = Src, .Version = Textures[Src].Version, .Dst = Dst });
	}

	bool RenderGraph::CreateHistoryTexture(Texture2** Dst, const TextureDesc2& Desc, const char* DebugName)
	{
		if (*Dst == nullptr || (*Dst)->GetDesc() != Desc)
		{
			TextureDesc2 DescCreate = Desc;
			DescCreate.Usage = DescCreate.Usage | TextureUsage::Sampled;

			// destroy the old one when safe
			if (*Dst != nullptr)
			{
				Device->DestroyTextureDeferred(*Dst);
			}

			*Dst = Device->CreateTexture(DescCreate);
			Device->SetDebugName(*Dst, DebugName);
			TextureInvalidations.push_back(RenderGraphHistoryInvalidation{ .Dst = *Dst });
			return true;
		}

		return false;
	}

	void RenderGraph::Clear()
	{
		PROFILE_CPU(Counter_RenderGraphClear);

		Passes.clear();
		Textures.clear();
		Buffers.clear();
		Extractions.clear();
		TextureInvalidations.clear();
		RenderData.ClearDescriptorData();

		// TODO: renderpass and framebuffer cleanup, framebuffer must be invalidated if any of texture that it depends on is invalidated

		// update resources usage flags, destroy previously unused resources
		for (auto& DescPool : TextureResourcePool)
		{
			TexturePool& Pool = DescPool.second;

			for (int i = 0; i < (int)Pool.size(); i++)
			{
				RenderGraphPooledTexture& PooledTexture = Pool[i];

				// unused textures cleanup logic
				if (PooledTexture.UnusedFrames > MaxFramesInFlight)
				{
					Texture2* Texture = Pool[i].Texture.get();

					Log::Message("RenderGraph texture deleted: %ix%ix%i", Texture->GetDesc().Width, Texture->GetDesc().Height, Texture->GetDesc().Depth);
					RemoveProfilingMemory(MemoryCounter_RenderGraphTextures, Texture->GetSize());

					Device->DestroyTexture(Texture);

					Pool.erase(Pool.begin() + i);
				}
				else
				{
					if (!PooledTexture.Used)
						PooledTexture.UnusedFrames++;

					PooledTexture.Used = false;
				}
			}
		}

		for (auto& DescPool : BufferResourcePool)
		{
			BufferPool& Pool = DescPool.second;

			for (int i = 0; i < (int)Pool.size(); i++)
			{
				RenderGraphPooledBuffer& PooledBuffer = Pool[i];

				// unused buffers cleanup logic
				if (PooledBuffer.UnusedFrames > MaxFramesInFlight)
				{
					Buffer* Buffer = Pool[i].Buffer.get();
					
					double Size;
					const char* Suffix = HumanizeBytes(Buffer->GetSize(), Size);

					Log::Message("RenderGraph buffer deleted: %0.2f %s", Size, Suffix);
					RemoveProfilingMemory(MemoryCounter_RenderGraphBuffers, Buffer->GetSize());

					Device->DestroyBuffer(Buffer);

					Pool.erase(Pool.begin() + i);
				}
				else
				{
					if (!PooledBuffer.Used)
						PooledBuffer.UnusedFrames++;

					PooledBuffer.Used = false;
				}
			}
		}

		Allocator.Clear();
		ExecutionHasFinished = false;
	}

	void RenderGraph::Build()
	{
		PROFILE_CPU(Counter_RenderGraphBuild);
		// TODO: topological sort and graph reordering

		// virtual resources allocation on build
		for (auto& Texture : Textures)
		{
			AllocateTexture(Texture);
		}

		for (auto& Buffer : Buffers)
		{
			AllocateBuffer(Buffer);
		}

		const auto UpdateTextureLifetime = [](RenderGraphTexture& Texture, RenderGraphPassId PassId) {
			if (PassId < Texture.FirstUsage) Texture.FirstUsage = PassId;
			if (PassId > Texture.LastUsage)  Texture.LastUsage  = PassId;
		};

		const auto UpdateBufferLifetime = [](RenderGraphBuffer& Buffer, RenderGraphPassId PassId) {
			if (PassId < Buffer.FirstUsage) Buffer.FirstUsage = PassId;
			if (PassId > Buffer.LastUsage)  Buffer.LastUsage  = PassId;
		};

		// derive lifetime information about resources
		// assuming that Passes is topologically sorted
		for (auto& Pass : Passes)
		{
			for (const auto& Read : Pass.Dependencies.TextureReadResources)
			{
				auto& GraphTexture = Textures[Read.Texture];

				// update resource lifetime
				UpdateTextureLifetime(GraphTexture, Pass.Id);

				// update resource consumers
				GraphTexture.Readers.push_back(GraphTexture.Id);
			}

			for (const auto& Read : Pass.Dependencies.BufferReadResources)
			{
				auto& GraphBuffer = Buffers[Read.Buffer];

				// update resource lifetime
				UpdateBufferLifetime(GraphBuffer, Pass.Id);

				// update resource consumers
				GraphBuffer.Readers.push_back(GraphBuffer.Id);
			}

			for (const auto& Write : Pass.Dependencies.TextureWriteResources)
			{
				auto& GraphTexture = Textures[Write.Texture];

				// update resource lifetime
				UpdateTextureLifetime(GraphTexture, Pass.Id);

				// update resource producer
				if (GraphTexture.Writer == -1)
				{
					GraphTexture.Writer = Pass.Id;
				}
				else
				{
					// TODO: proper validation
					// Log::Error("RenderGraph Validation: Texture %s can have only one producer, %s writes over it, was written in %s before",
					// 	GraphTexture.DebugName.c_str(), Pass.Name.c_str(), Passes[GraphTexture.Writer].Name.c_str());
				}
			}

			for (const auto& Write : Pass.Dependencies.BufferWriteResources)
			{
				auto& GraphBuffer = Buffers[Write.Buffer];

				// update resource lifetime
				UpdateBufferLifetime(GraphBuffer, Pass.Id);

				// update resource producer
				if (GraphBuffer.Writer == -1)
				{
					GraphBuffer.Writer = Pass.Id;
				}
				else
				{
					// TODO: proper validation
					// Log::Error("RenderGraph Validation: Buffer %s can have only one producer, %s writes over it, was written in %s before",
					// 	GraphBuffer.DebugName.c_str(), Pass.Name.c_str(), Passes[GraphBuffer.Writer].Name.c_str());
				}
			}
		}

		// Validate resources
		for (auto& Texture : Textures)
		{
			// it is illegal to read from a resource that hasn't been written before
			if (Texture.Readers.size() > 0 && Texture.Writer == -1)
			{
				Log::Error("RenderGraph Validation: Texture %s is being read but no passes have ever written to it", Texture.DebugName.c_str());
			}
		}

		for (auto& Buffer : Buffers)
		{
			// it is illegal to read from a resource that hasn't been written before
			if (Buffer.Readers.size() > 0 && Buffer.Writer == -1)
			{
				Log::Error("RenderGraph Validation: Buffer %s is being read but no passes have ever written to it", Buffer.DebugName.c_str());
			}
		}

		// knowing resource adjacent passes (writer and readers), setup barriers for a pass
		// needed only for reading passes
		for (auto& Pass : Passes)
		{
			for (const auto& Read : Pass.Dependencies.TextureReadResources)
			{
				const auto& GraphTexture = Textures[Read.Texture];
				const auto& WriterPass = Passes[GraphTexture.Writer];

				// search for the writing dependency, TODO: better solution?
				for (const auto& WriterPassWrite : WriterPass.Dependencies.TextureWriteResources)
				{
					if (WriterPassWrite.Texture == Read.Texture)
					{
						Pass.TextureBarriers.push_back(RenderGraphTextureBarrier { GraphTexture.Id, WriterPassWrite, Read });
					}
				}
			}

			for (const auto& Read : Pass.Dependencies.BufferReadResources)
			{
				const auto& GraphBuffer = Textures[Read.Buffer];
				const auto& WriterPass = Passes[GraphBuffer.Writer];

				// search for the writing dependency, TODO: better solution?
				for (const auto& WriterPassWrite : WriterPass.Dependencies.BufferWriteResources)
				{
					if (WriterPassWrite.Buffer == Read.Buffer)
					{
						Pass.BufferBarriers.push_back(RenderGraphBufferBarrier { GraphBuffer.Id, WriterPassWrite, Read });
					}
				}
			}
		}

		// test: dump resources lifetime information, TODO: expose into debug dump
		#if 0
		for (auto& Texture : Textures)
		{
			printf("Texture lifetime: %s lives between %s and %s\n", Texture.DebugName.c_str(), Passes[Texture.FirstUsage].Name.c_str(), Passes[Texture.LastUsage].Name.c_str());
		}

		for (auto& Buffer : Buffers)
		{
			printf("Buffer lifetime: %s lives between %s and %s\n", Buffer.DebugName.c_str(), Passes[Buffer.FirstUsage].Name.c_str(), Passes[Buffer.LastUsage].Name.c_str());
		}
		#endif

		// test: dump scheduling and barriers information, TODO: expose into debug dump
		#if 0
		{
			printf("RenderGraph Scheduling: ");
			for (auto& Pass : Passes)
			{
				printf("%s (texture barriers:", Pass.Name.c_str());
				for (auto& Barrier : Pass.TextureBarriers)
				{
					printf(" %s_v%i,", Textures[Barrier.Texture].DebugName.c_str(), Barrier.Reader.Version);
				}
				printf("),");

				printf(" (buffer barriers:");
				for (auto& Barrier : Pass.BufferBarriers)
				{
					printf(" %s_v%i,", Buffers[Barrier.Buffer].DebugName.c_str(), Barrier.Reader.Version);
				}
				printf("),");
			}
			printf("\n");
		}
		#endif

		// some vulkan-specific stuff for renderpasses
		for (RenderPass& Pass : Passes)
		{
			if (Pass.Type == RenderGraphPassType::Raster)
			{
				if (Pass.Parameters.ViewportSize == iVector2(-1))
				{
					Pass.Parameters.ViewportSize = RenderData.DefaultViewportSize;
				}

				RenderGraphPassParametersRHI RHIParams;

				// convert Pass.Parameters into RHIParams
				{
					RHIParams.Size = Pass.Parameters.ViewportSize;

					for (auto& ColorAttachment : Pass.Parameters.ColorAttachments)
					{
						if (ColorAttachment)
						{
							Texture2* Texture = Textures[ColorAttachment->Texture].Texture.get();
							TextureFormat Format = Texture->GetDesc().Format;

							RHIParams.AttachmentDescs[RHIParams.NumUsedAttachments] = AttachmentDesc(AttachmentType::Color, ColorAttachment->LoadOp, Format);
							RHIParams.AttachmentTextures[RHIParams.NumUsedAttachments] = Texture;
							RHIParams.NumUsedAttachments++;
						}
					}

					if (Pass.Parameters.DepthStencilAttachment)
					{
						auto Attachment = Pass.Parameters.DepthStencilAttachment;
						Texture2* Texture = Textures[Attachment->Texture].Texture.get();

						RHIParams.AttachmentDescs[RHIParams.NumUsedAttachments] = AttachmentDesc(AttachmentType::DepthStencil, Attachment->LoadOp, Texture->GetDesc().Format);
						RHIParams.AttachmentTextures[RHIParams.NumUsedAttachments] = Texture;
						RHIParams.NumUsedAttachments++;
					}
				}

				Pass.VulkanRenderPass = Pass.Parameters.ExternalRenderPass == NULL ? GetOrCreateVulkanRenderPass(RHIParams) : Pass.Parameters.ExternalRenderPass;
				Pass.VulkanFramebuffer = GetOrCreateVulkanFramebuffer(RHIParams, Pass.VulkanRenderPass);
			}
		}

		// update CPU memory stats
		SetProfilingMemory(MemoryCounter_RenderGraphCPU, 0);
		AddProfilingMemory(MemoryCounter_RenderGraphCPU, Passes.size() * sizeof(RenderPass));
		AddProfilingMemory(MemoryCounter_RenderGraphCPU, Textures.size() * sizeof(RenderGraphTexture));
		AddProfilingMemory(MemoryCounter_RenderGraphCPU, Buffers.size() * sizeof(RenderGraphBuffer));
		AddProfilingMemory(MemoryCounter_RenderGraphCPU, Extractions.size() * sizeof(RenderGraphTextureExtraction));
		AddProfilingMemory(MemoryCounter_RenderGraphCPU, TextureInvalidations.size() * sizeof(RenderGraphHistoryInvalidation));
		AddProfilingMemory(MemoryCounter_RenderGraphCPU, MarkersStack.size() * sizeof(RenderGraphMarker));
		AddProfilingMemory(MemoryCounter_RenderGraphCPU, sizeof(RenderGraphData));
		AddProfilingMemory(MemoryCounter_RenderGraphCPU, Allocator.GetTotalAllocated());

		SetProfilingMemory(MemoryCounter_RenderGraphAllocatorWaste, Allocator.CurrentOffset - Allocator.AttemptsToDeallocate);
	}

	RenderGraphExecuteResults RenderGraph::Execute(const RenderGraphExecuteParameters& Parameters)
	{
		RenderData.CurrentPerFrameData = (RenderData.CurrentPerFrameData + 1) % MaxFramesInFlight;
		auto& PerFrameData = RenderData.PerFrameData[RenderData.CurrentPerFrameData];
		PerFrameData.CurrentCmdBuffer = -1;

		assert(Parameters.DefaultViewportSize.X > 0 && Parameters.DefaultViewportSize.Y > 0);
		RenderData.DefaultViewportSize = Parameters.DefaultViewportSize;

		PerFrameData.FirstSemaphore = Parameters.WaitSemaphore;

		// TODO: refactor that mechanism
		// TODO: move to GPUScene
		if (Scene)
		{
			for (int i = 0; i < Scene->Textures.size(); i++)
			{
				Device->UpdateDescriptorSet(RenderData.GPUSceneData.TextureSet, 0, i, Scene->Textures[i], TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				Device->UpdateDescriptorSet(RenderData.GPUSceneData.TextureSetNonCombined, 0, i, Scene->Textures[i], TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			}
		}

		Build();

		PROFILE_CPU(Counter_RenderGraphExecute);

		int CmdBufferSubmissions = 1;
		for (RenderPass& Pass : Passes)
		{
			if (Pass.Parameters.SubmitBeforeExecution)
			{
				CmdBufferSubmissions++;
			}
		}
		// CmdBufferSubmissions - 1 == number of in-between semaphores

		auto CommandBuffer = PerFrameData.GetNextCommandBuffer(Device);
		auto WaitSemaphore = PerFrameData.GetPreviousSemaphore(Device, CmdBufferSubmissions);
		auto SignalSemaphore = PerFrameData.GetNextSemaphore(Device, CmdBufferSubmissions);

		// TODO: multithreaded rendering
		RenderGraphContext Context{NULL, -1, Device, Scene, CommandBuffer, RenderData, *this};
		CommandBuffer->Reset();
		CommandBuffer->Begin();

		// TODO: move profiler calls out from rendergraph
		ProfileMarkerGPU GpuTimeMarker(&GpuCounter_RenderGraphFrame);
		Context.Device->_Profiler.Reset(CommandBuffer);
		Context.Device->_Profiler.BeginProfileCounter(GpuTimeMarker, CommandBuffer);

		auto EvaluateMarker = [this, &CommandBuffer]()
		{
			RenderGraphMarker Marker = MarkersStack.front();
			switch (Marker.Type)
			{
				case RenderGraphMarker::TypePush: CommandBuffer->BeginDebugMarker(Marker.Marker); break;
				case RenderGraphMarker::TypePop:  CommandBuffer->EndDebugMarker(); break;
				default: COLUMBUS_ASSERT_MESSAGE(false, "Invalid marker"); break;
			}
			MarkersStack.pop();
		};

		// history invalidations
		if (TextureInvalidations.size() > 0)
		{
			CommandBuffer->BeginDebugMarker("History Invalidations");
			for (RenderGraphHistoryInvalidation& Invalidation : TextureInvalidations)
			{
				CommandBuffer->MemsetTexture(Invalidation.Dst, Vector4(0, 0, 0, 0));
			}
			CommandBuffer->EndDebugMarker();
		}

		// graph execution
		for (auto& Pass : Passes)
		{
			if (Pass.Parameters.SubmitBeforeExecution)
			{
				CommandBuffer->End();

				Device->Submit(CommandBuffer, nullptr, 1, &WaitSemaphore, 1, &SignalSemaphore);

				CommandBuffer = PerFrameData.GetNextCommandBuffer(Device);
				WaitSemaphore = PerFrameData.GetPreviousSemaphore(Device, CmdBufferSubmissions);
				SignalSemaphore = PerFrameData.GetNextSemaphore(Device, CmdBufferSubmissions);

				Context.CommandBuffer = CommandBuffer;
				CommandBuffer->Reset();
				CommandBuffer->Begin();
			}

			Context.CurrentPass = Pass.Id;

			while (!MarkersStack.empty() && MarkersStack.front().PassId == Pass.Id)
			{
				EvaluateMarker();
			}

			for (RenderPassTextureDependency& TexWrite : Pass.Dependencies.TextureWriteResources)
			{
				Textures[TexWrite.Texture].CurrentVersion++;
			}

			for (RenderPassBufferDependency& BufWrite : Pass.Dependencies.BufferWriteResources)
			{
				Buffers[BufWrite.Buffer].CurrentVersion++;
			}

			// pass marker
			CommandBuffer->BeginDebugMarker(Pass.Name);

			// issue barriers for this pass
			for (const auto& Barrier : Pass.TextureBarriers)
			{
				const RenderGraphTexture& GraphTexture = Textures[Barrier.Texture];

				TextureVulkan* Texture = static_cast<TextureVulkan*>(GraphTexture.Texture.get());
				VkImageAspectFlags AspectFlags = TextureFormatToAspectMaskVk(Texture->GetDesc().Format);

				VkImageMemoryBarrier VkBarrier;
				VkBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				VkBarrier.pNext = nullptr;
				VkBarrier.srcAccessMask = Barrier.Writer.Access;
				VkBarrier.dstAccessMask = Barrier.Reader.Access;
				VkBarrier.oldLayout = Texture->_Layout;
				VkBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
				VkBarrier.srcQueueFamilyIndex = 0;
				VkBarrier.dstQueueFamilyIndex = 0;
				VkBarrier.image = Texture->_Image;
				VkBarrier.subresourceRange.aspectMask = AspectFlags;
				VkBarrier.subresourceRange.baseMipLevel = 0;
				VkBarrier.subresourceRange.levelCount = 1;
				VkBarrier.subresourceRange.baseArrayLayer = 0;
				VkBarrier.subresourceRange.layerCount = 1;

				vkCmdPipelineBarrier(CommandBuffer->_CmdBuf, Barrier.Writer.Stage, Barrier.Reader.Stage, 0,
					0, 0, // memory barrier
					0, 0, // buffer memory barrier
					1, (VkImageMemoryBarrier*)&VkBarrier);

				Texture->_Layout = VK_IMAGE_LAYOUT_GENERAL; // TODO: proper tracking system
			}

			for (const auto& Barrier : Pass.BufferBarriers)
			{
				const RenderGraphBuffer& GraphBuffer = Buffers[Barrier.Buffer];

				BufferVulkan* Buffer = static_cast<BufferVulkan*>(GraphBuffer.Buffer.get());

				VkBufferMemoryBarrier VkBarrier;
				VkBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				VkBarrier.pNext = nullptr;
				VkBarrier.srcAccessMask = Barrier.Writer.Access;
				VkBarrier.dstAccessMask = Barrier.Reader.Access;
				VkBarrier.srcQueueFamilyIndex = 0;
				VkBarrier.dstQueueFamilyIndex = 0;
				VkBarrier.buffer = Buffer->_Buffer;
				VkBarrier.offset = 0; // TODO?
				VkBarrier.size = GraphBuffer.Desc.Size;

				vkCmdPipelineBarrier(CommandBuffer->_CmdBuf, Barrier.Writer.Stage, Barrier.Reader.Stage, 0,
					0, 0, // memory barrier
					1, &VkBarrier, // buffer memory barrier
					0, 0  // image memory barrier
				);
			}

			if (Pass.Type == RenderGraphPassType::Raster)
			{
				fixed_vector<VkClearValue, 16> ClearValues;
				for (const auto& Attachment : Pass.Parameters.ColorAttachments)
				{
					if (Attachment && Attachment->LoadOp == AttachmentLoadOp::Clear)
					{
						ClearValues.push_back(AttachmentClearValueToVk(Attachment->ClearValue, false));
					}
				}

				if (Pass.Parameters.DepthStencilAttachment && Pass.Parameters.DepthStencilAttachment->LoadOp == AttachmentLoadOp::Clear)
				{
					ClearValues.push_back(AttachmentClearValueToVk(Pass.Parameters.DepthStencilAttachment->ClearValue, true));
				}

				Context.VulkanRenderPass = Pass.VulkanRenderPass;

				// TODO: resize, extent
				VkRect2D PassRect {
					.offset { .x = 0, .y = 0 },
					.extent {
						.width = (u32)Pass.Parameters.ViewportSize.X,
						.height = (u32)Pass.Parameters.ViewportSize.Y,
					},
				};

				CommandBuffer->BeginRenderPass(Pass.VulkanRenderPass, PassRect, Pass.VulkanFramebuffer, (u32)ClearValues.size(), ClearValues.data());
				CommandBuffer->SetViewport(0, 0, (float)Pass.Parameters.ViewportSize.X, (float)Pass.Parameters.ViewportSize.Y, 0, 1);
				CommandBuffer->SetScissor(0, 0, (u32)Pass.Parameters.ViewportSize.X, (u32)Pass.Parameters.ViewportSize.Y);
				Pass.ExecutionFunc(Context);
				CommandBuffer->EndRenderPass();

				{
					// raster pass transitions attachments layouts
					// TODO: better system?
					for (auto& Attachment : Pass.Parameters.ColorAttachments)
					{
						if (Attachment && Textures[Attachment->Texture].Texture)
						{
							TextureVulkan* Texture = static_cast<TextureVulkan*>(Textures[Attachment->Texture].Texture.get());
							if (Texture != nullptr)
							{
								Texture->_Layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
							}
						}
					}

					if (Pass.Parameters.DepthStencilAttachment)
					{
						TextureVulkan* Texture = static_cast<TextureVulkan*>(Textures[Pass.Parameters.DepthStencilAttachment->Texture].Texture.get());
						if (Texture != nullptr)
						{
							Texture->_Layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
						}
					}
				}
			}
			else
			{
				Pass.ExecutionFunc(Context);
			}

			// pass marker
			CommandBuffer->EndDebugMarker();
		}

		// evaluate all last markers
		while (!MarkersStack.empty())
		{
			EvaluateMarker();
		}

		// extractions
		CommandBuffer->BeginDebugMarker("Extract");
		for (RenderGraphTextureExtraction& Extraction : Extractions)
		{
			if (Extraction.Version == Textures[Extraction.Src].CurrentVersion)
			{
				CommandBuffer->BeginDebugMarker(Textures[Extraction.Src].DebugName.c_str());

				if (*Extraction.Dst == nullptr || (*Extraction.Dst != nullptr && (*Extraction.Dst)->GetDesc() != Textures[Extraction.Src].Desc))
				{
					// TODO: move to allocation phase?
					// TODO: resize invalidation
					*Extraction.Dst = Device->CreateTexture(Textures[Extraction.Src].Desc);
				}

				iVector3 Size = { (int)Textures[Extraction.Src].Desc.Width, (int)Textures[Extraction.Src].Desc.Height, (int)Textures[Extraction.Src].Desc.Depth };

				auto SrcLayout = static_cast<TextureVulkan*>(Textures[Extraction.Src].Texture.get())->_Layout;
				auto DstLayout = static_cast<TextureVulkan*>(*Extraction.Dst)->_Layout;

				// TODO: layout management
				CommandBuffer->TransitionImageLayout(Textures[Extraction.Src].Texture.get(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				CommandBuffer->TransitionImageLayout(*Extraction.Dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				CommandBuffer->CopyImage(Textures[Extraction.Src].Texture.get(), *Extraction.Dst, {}, {}, Size);
				CommandBuffer->TransitionImageLayout(Textures[Extraction.Src].Texture.get(), SrcLayout);
				//CommandBuffer->TransitionImageLayout(*Extraction.Dst, DstLayout);
				CommandBuffer->TransitionImageLayout(*Extraction.Dst, VK_IMAGE_LAYOUT_GENERAL);
				CommandBuffer->EndDebugMarker();
			}
		}
		CommandBuffer->EndDebugMarker();

		Context.Device->_Profiler.EndProfileConter(GpuTimeMarker, CommandBuffer);
		CommandBuffer->End();
		Device->Submit(CommandBuffer, Parameters.SignalFence, 1, &WaitSemaphore, 1, &SignalSemaphore);

		RenderGraphExecuteResults Result;
		Result.FinishSemaphore = SignalSemaphore;

		ExecutionHasFinished = true;

		return Result;
	}

	std::string RenderGraph::ExportGraphviz()
	{
		// TODO: colour-mark nodes

		std::string Result;

		Result += "digraph G {\n";

		for (const auto& Pass : Passes)
		{
			Result += std::string(Pass.Name) + " [shape=circle]\n";
		}

		for (const auto& Texture : Textures)
		{
			for (int Version = 0; Version <= Texture.Version; Version++)
			{
				Result += Texture.DebugName + "_v" + std::to_string(Version) + " [shape=box]\n";
			}
		}

		for (const auto& Buffer : Buffers)
		{
			for (int Version = 0; Version <= Buffer.Version; Version++)
			{
				Result += Buffer.DebugName + + "_v" + std::to_string(Version) + " [shape=box]\n";
			}
		}

		for (const auto& Pass : Passes)
		{
			for (const auto& Read : Pass.Dependencies.TextureReadResources)
			{
				Result += Textures[Read.Texture].DebugName + "_v" + std::to_string(Read.Version) + " -> " + Pass.Name + "\n";
			}

			for (const auto& Read : Pass.Dependencies.BufferReadResources)
			{
				Result += Buffers[Read.Buffer].DebugName + "_v" + std::to_string(Read.Version) + " -> " + Pass.Name + "\n";
			}

			for (const auto& Write : Pass.Dependencies.TextureWriteResources)
			{
				Result += std::string(Pass.Name) + " -> " + Textures[Write.Texture].DebugName + "_v" + std::to_string(Write.Version) + "\n";
			}

			for (const auto& Write : Pass.Dependencies.BufferWriteResources)
			{
				Result += std::string(Pass.Name) + " -> " + Buffers[Write.Buffer].DebugName + "_v" + std::to_string(Write.Version) + "\n";
			}
		}

		Result += "}\n";

		return Result;
	}

	void RenderGraph::PrintDebugInformation()
	{
		// TODO
	}

	void RenderGraph::GetDebugInformation(RenderGraphDebugInformation& Info)
	{
		Info.Passes = Passes;
		Info.Textures = Textures;
		Info.Buffers = Buffers;
	}

	// TODO: destroy unused renderpasses
	VkRenderPass RenderGraph::GetOrCreateVulkanRenderPass(RenderGraphPassParametersRHI& AttachmentParams)
	{
		for (const auto& ParamsPassPair : VulkanRenderPasses)
		{
			if (ParamsPassPair.first.EqualsDescs(AttachmentParams))
				return ParamsPassPair.second;
		}

		Log::Message("Call: CreateRenderPass");

		std::pair<RenderGraphPassParametersRHI, VkRenderPass> NewPair = {
			AttachmentParams,
			Device->CreateRenderPass(std::span<AttachmentDesc>(AttachmentParams.AttachmentDescs, AttachmentParams.NumUsedAttachments))
		};

		VulkanRenderPasses.push_back(NewPair);
		return NewPair.second;
	}

	// TODO: invalidate framebuffers when texture is destroyed
	// TODO: destroy unused framebuffers
	VkFramebuffer RenderGraph::GetOrCreateVulkanFramebuffer(RenderGraphPassParametersRHI& AttachmentParams, VkRenderPass RenderPassVulkan)
	{
		for (const auto& ParamsFramebufferPair : VulkanFramebuffers)
		{
			if (ParamsFramebufferPair.first.EqualsDescsAndTexturesAndSize(AttachmentParams))
				return ParamsFramebufferPair.second;
		}

		Log::Message("Call: CreateFramebuffer: %ix%i", AttachmentParams.Size.X, AttachmentParams.Size.Y);

		std::pair<RenderGraphPassParametersRHI, VkFramebuffer> NewPair = {
			AttachmentParams,
			Device->CreateFramebuffer(RenderPassVulkan, AttachmentParams.Size, std::span<Texture2*>(AttachmentParams.AttachmentTextures, AttachmentParams.NumUsedAttachments))
		};

		VulkanFramebuffers.push_back(NewPair);
		return NewPair.second;
	}

}
