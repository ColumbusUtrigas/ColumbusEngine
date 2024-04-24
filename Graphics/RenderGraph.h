#pragma once

#include <Core/Core.h>
#include <Graphics/Core/GraphicsCore.h>
#include <Graphics/Vulkan/DeviceVulkan.h>
#include <Profiling/Profiling.h>
#include "GPUScene.h"
#include "Graphics/Vulkan/CommandBufferVulkan.h"
#include "Graphics/Vulkan/FenceVulkan.h"
#include "Graphics/Vulkan/SwapchainVulkan.h"
#include <memory>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <optional>

#include <vulkan/vulkan.h>

// to be used inside of a pass executor
#define RENDER_GRAPH_PROFILE_GPU_SCOPED(counter, context) PROFILE_GPU(counter, &context.Device->_Profiler, context.CommandBuffer);

DECLARE_CPU_PROFILING_COUNTER(Counter_RenderGraphClear);
DECLARE_CPU_PROFILING_COUNTER(Counter_RenderGraphBuild);
DECLARE_CPU_PROFILING_COUNTER(Counter_RenderGraphExecute);

DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_RenderGraphTextures);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_RenderGraphBuffers);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_RenderGraphCPU);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_RenderGraphAllocatorWaste);

DECLARE_GPU_PROFILING_COUNTER(GpuCounter_RenderGraphFrame);

namespace Columbus
{

	// forward declarations
	class RenderGraph;
	struct RenderGraphContext;
	struct RenderGraphTexture;

	using RenderGraphExecutionFunc = std::function<void(RenderGraphContext&)>;
	using RenderGraphTextureRef = int; // TODO: unify with Id
	using RenderGraphTextureId = int;
	using RenderGraphBufferRef = int; // TODO: unify with Id
	using RenderGraphBufferId = int;
	using RenderGraphPassId = int;

	static constexpr int RenderGraphMemoryPoolSize = 1024*1024;

	// per-frame allocator of temporary resources, specifically for RenderGraph
	// all pointers are valid only during frame graph build and execution
	struct RenderGraphAllocator
	{
		const size_t PoolSize;
		size_t CurrentOffset;
		size_t AttemptsToDeallocate;
		void* Memory;

		RenderGraphAllocator(size_t PoolSize) : PoolSize(PoolSize)
		{
			Memory = malloc(PoolSize);
			Clear();
		}

		void* Allocate(size_t Bytes)
		{
			COLUMBUS_ASSERT_MESSAGE((CurrentOffset + Bytes) <= PoolSize, "Ran out of RenderGraph pool memory");
			void* Pointer = (char*)Memory + CurrentOffset;
			CurrentOffset += Bytes;
			return Pointer;
		}

		char* StrDup(const char* SrcStr)
		{
			size_t len = strlen(SrcStr);
			char* str = (char*)Allocate(len+1);
			memcpy(str, SrcStr, len+1);
			return str;
		}

		void Free(void* Ptr)
		{
			// doesn't free
		}

		void Clear()
		{
			CurrentOffset = 0;
			AttemptsToDeallocate = 0;
		}

		size_t GetTotalSize() const { return PoolSize; }
		size_t GetTotalAllocated() const { return CurrentOffset; }

		~RenderGraphAllocator()
		{
			free(Memory);
			Memory = nullptr;
		}
	};

	// a wrapper to use allocator with std containers
	template <typename T>
	struct RenderGraphAllocatorStd
	{
		using value_type = T;
		using size_type = size_t;

		RenderGraphAllocator& Allocator;

		RenderGraphAllocatorStd(RenderGraphAllocator& Allocator) : Allocator(Allocator) {}
		RenderGraphAllocatorStd(const RenderGraphAllocatorStd& A) : Allocator(A.Allocator) {}

		template <typename U>
		RenderGraphAllocatorStd(const RenderGraphAllocatorStd<U>& A) : Allocator(A.Allocator) {}

		T* allocate(size_t n) noexcept
		{
			return (T*)Allocator.Allocate(sizeof(T) * n);
		}

		void deallocate(T* p, size_t s) noexcept
		{
			Allocator.AttemptsToDeallocate += sizeof(T) * s;
			return Allocator.Free(p);
		}
	};

	template <typename T>
	using RenderGraphDynamicArray = std::vector<T, RenderGraphAllocatorStd<T>>;

	struct RenderGraphData
	{
		struct
		{
			VkSemaphore FirstSemaphore; // external
			VkSemaphore SubmitSemaphore;

			int CurrentCmdBuffer = -1; // reset every frame

			std::vector<CommandBufferVulkan*> CommandBuffers;
			std::vector<VkSemaphore> CommandBuffersSemaphores; // placed between 2 subsequent cmd buffers

			CommandBufferVulkan* GetNextCommandBuffer(SPtr<DeviceVulkan> Device)
			{
				CurrentCmdBuffer++;
				if (CurrentCmdBuffer >= CommandBuffers.size())
				{
					CommandBufferVulkan* CmdBuf = Device->CreateCommandBuffer();
					CommandBuffers.push_back(CmdBuf);
				}

				return CommandBuffers[CurrentCmdBuffer];
			}

			VkSemaphore InternalGetSemaphore(SPtr<DeviceVulkan> Device, int Id)
			{
				if (Id >= CommandBuffersSemaphores.size())
				{
					VkSemaphore Semaphore = Device->CreateSemaphore();
					CommandBuffersSemaphores.push_back(Semaphore);
				}

				return CommandBuffersSemaphores[Id];
			}

			// call these after GetNextCommandBuffer!
			VkSemaphore GetPreviousSemaphore(SPtr<DeviceVulkan> Device, int TotalNumberOfSubmissions)
			{
				if (CurrentCmdBuffer == 0)
				{
					return FirstSemaphore;
				}

				return InternalGetSemaphore(Device, CurrentCmdBuffer - 1);
			}

			VkSemaphore GetNextSemaphore(SPtr<DeviceVulkan> Device, int TotalNumberOfSubmissions)
			{
				if (CurrentCmdBuffer == TotalNumberOfSubmissions - 1)
				{
					return SubmitSemaphore;
				}

				return InternalGetSemaphore(Device, CurrentCmdBuffer);
			}

		} PerFrameData[MaxFramesInFlight];

		// TODO: move to shader caching system
		// TODO: proper descriptor set management, currently if two passes use one shader, they overlap
		struct PipelineDescriptorSetData
		{
			VkDescriptorSet DescriptorSets[16] {0};
		};

		// TODO: move to GPUScene
		struct
		{
			VkDescriptorSetLayout TextureLayout;
			VkDescriptorSetLayout SceneLayout;
		} GPUSceneLayout;

		// TODO: move to GPUScene
		struct
		{
			VkDescriptorSet TextureSet;
			VkDescriptorSet SceneSet;
		} GPUSceneData;

		// TODO: move to shader caching system
		std::unordered_map<VkPipeline, PipelineDescriptorSetData> DescriptorSets[MaxFramesInFlight];

		int CurrentPerFrameData = 0;
		iVector2 DefaultViewportSize;
	};

	struct RenderGraphContext
	{
		VkRenderPass VulkanRenderPass; // TODO: remove
		RenderGraphPassId CurrentPass;
		SPtr<DeviceVulkan> Device;
		SPtr<GPUScene> Scene;
		CommandBufferVulkan* CommandBuffer;
		RenderGraphData& RenderData;
		RenderGraph& Graph;

		// TODO: separate shader binding system?
		VkDescriptorSet GetDescriptorSet(const ComputePipeline* Pipeline, int Index);
		VkDescriptorSet GetDescriptorSet(const GraphicsPipeline* Pipeline, int Index);
		VkDescriptorSet GetDescriptorSet(const RayTracingPipeline* Pipeline, int Index);

		void BindGPUScene(const GraphicsPipeline* Pipeline);
		void BindGPUScene(const RayTracingPipeline* Pipeline);

		SPtr<Texture2> GetRenderGraphTexture(RenderGraphTextureRef Ref);
		SPtr<Buffer> GetRenderGraphBuffer(RenderGraphBufferRef Ref);
	};

	struct RenderPassAttachment
	{
		AttachmentLoadOp LoadOp;
		RenderGraphTextureRef Texture;
		AttachmentClearValue ClearValue;

		bool operator==(const RenderPassAttachment&) const = default;
	};

	struct RenderPassParameters
	{
		static constexpr int ColorAttachmentsCount = 8;

		std::optional<RenderPassAttachment> ColorAttachments[ColorAttachmentsCount];
		std::optional<RenderPassAttachment> DepthStencilAttachment;

		bool SubmitBeforeExecution = false;

		// if you want to create render pass externally and use it in the graph
		VkRenderPass ExternalRenderPass = NULL;

		// raster only
		iVector2 ViewportSize{-1};
	};

	// RenderPassParameters is converted into this struct during rendergraph build
	// for internal use
	struct RenderGraphPassParametersRHI
	{
		static constexpr int AttachmentsMax = RenderPassParameters::ColorAttachmentsCount + 1; // +1 because of DepthStencil attachment

		AttachmentDesc AttachmentDescs[AttachmentsMax]{}; // will be used to create RHI render passes and framebuffers
		Texture2* AttachmentTextures[AttachmentsMax]{ nullptr };
		iVector2 Size;
		int NumUsedAttachments = 0;

		bool EqualsDescs(const RenderGraphPassParametersRHI& Other) const
		{
			if (Other.NumUsedAttachments != NumUsedAttachments)
				return false;

			for (int i = 0; i < NumUsedAttachments; i++)
			{
				if (AttachmentDescs[i] != Other.AttachmentDescs[i])
					return false;
			}

			return true;
		}

		bool EqualsDescsAndTexturesAndSize(const RenderGraphPassParametersRHI& Other) const
		{
			if (!EqualsDescs(Other))
				return false;

			if (Size != Other.Size)
				return false;

			for (int i = 0; i < NumUsedAttachments; i++)
			{
				if (AttachmentTextures[i] != Other.AttachmentTextures[i])
					return false;
			}

			return true;
		}
	};

	struct HashRenderPassParameters
	{
		size_t operator()(const RenderPassParameters& Params) const;
	};

	enum class RenderGraphPassType
	{
		Raster,
		Compute,
		// TODO: AsyncCompute
	};

	// TODO: unify buffer and texture internally as a resource?
	struct RenderGraphTexture
	{
		SPtr<Texture2> Texture;
		TextureDesc2 Desc;

		size_t AllocatedSize = 0;

		std::string DebugName; // TODO: more efficient data storage
		RenderGraphTextureId Id;

		// information about adjacent passes, needed to setup barriers
		// TODO: better solution to get a specific dependency (edge)?
		int Writer = -1; // can only be one writer
		RenderGraphDynamicArray<int> Readers;

		// resource lifetime information
		RenderGraphPassId FirstUsage = INT_MAX;
		RenderGraphPassId LastUsage = INT_MIN;

		// Passes can overwrite a texture, incrementing a version
		int Version = -1; // incremented during build
		int CurrentVersion = -1; // incremented during execution

		RenderGraphTexture(const TextureDesc2& Desc, std::string_view Name, RenderGraphTextureId Id, RenderGraphAllocator& Allocator)
			: Desc(Desc), DebugName(Name), Id(Id), Readers(Allocator) {}
	};

	// TODO: unify buffer and texture internally as a resource?
	struct RenderGraphBuffer
	{
		SPtr<Buffer> Buffer;
		BufferDesc Desc;

		size_t AllocatedSize = 0;

		std::string DebugName; // TODO: more efficient data storage
		RenderGraphBufferId Id;

		// information about adjacent passes, needed to setup barriers
		// TODO: better solution to get a specific dependency (edge)?
		int Writer = -1; // can only be one writer
		RenderGraphDynamicArray<int> Readers;

		// resource lifetime information
		RenderGraphPassId FirstUsage = INT_MAX;
		RenderGraphPassId LastUsage = INT_MIN;

		// Passes can overwrite a texture, incrementing a version
		int Version = -1; // incremented during build
		int CurrentVersion = -1; // incremented during execution

		RenderGraphBuffer(const BufferDesc& Desc, std::string_view Name, RenderGraphBufferId Id, RenderGraphAllocator& Allocator)
			: Desc(Desc), DebugName(Name), Id(Id), Readers(Allocator) {}
	};

	// TODO: unify buffer and texture dependencies internally?
	struct RenderPassTextureDependency
	{
		RenderGraphTextureId Texture;
		int Version = -1;

		// TODO?
		VkAccessFlags Access;
		VkPipelineStageFlags Stage;
	};

	// TODO: unify buffer and texture dependencies internally?
	struct RenderPassBufferDependency
	{
		RenderGraphBufferId Buffer;
		int Version = -1;

		// TODO?
		VkAccessFlags Access;
		VkPipelineStageFlags Stage;
	};

	struct RenderPassDependencies
	{
		RenderPassDependencies(RenderGraphAllocator& Allocator) :
			TextureWriteResources(Allocator), TextureReadResources(RenderGraphAllocatorStd<RenderPassTextureDependency>(Allocator)),
			BufferWriteResources(Allocator), BufferReadResources(RenderGraphAllocatorStd<RenderPassBufferDependency>(Allocator)) {}

		void Write(RenderGraphTextureId Texture, VkAccessFlags Access, VkPipelineStageFlags Stage)
		{
			TextureWriteResources.push_back(RenderPassTextureDependency { Texture, -1, Access, Stage });
		}

		void Read(RenderGraphTextureId Texture, VkAccessFlags Access, VkPipelineStageFlags Stage)
		{
			TextureReadResources.push_back(RenderPassTextureDependency { Texture, -1, Access, Stage });
		}

		void WriteBuffer(RenderGraphBufferId Buffer, VkAccessFlags Access, VkPipelineStageFlags Stage)
		{
			BufferWriteResources.push_back(RenderPassBufferDependency { Buffer, -1, Access, Stage });
		}

		void ReadBuffer(RenderGraphBufferId Buffer, VkAccessFlags Access, VkPipelineStageFlags Stage)
		{
			BufferReadResources.push_back(RenderPassBufferDependency { Buffer, -1, Access, Stage });
		}
	private:
		RenderGraphDynamicArray<RenderPassTextureDependency> TextureWriteResources;
		RenderGraphDynamicArray<RenderPassTextureDependency> TextureReadResources;

		RenderGraphDynamicArray<RenderPassBufferDependency> BufferWriteResources;
		RenderGraphDynamicArray<RenderPassBufferDependency> BufferReadResources;

		friend RenderGraph;
		friend RenderGraphContext;
	};

	// TODO: unify textures and buffers internally?
	struct RenderGraphTextureBarrier
	{
		RenderGraphTextureId Texture;
		RenderPassTextureDependency Writer; // (to extract vk flags from)
		RenderPassTextureDependency Reader; // (to extract vk flags from)
	};

	// TODO: unify textures and buffers internally?
	struct RenderGraphBufferBarrier
	{
		RenderGraphBufferId Buffer;
		RenderPassBufferDependency Writer; // (to extract vk flags from)
		RenderPassBufferDependency Reader; // (to extract vk flags from)
	};

	struct RenderPass
	{
		const char* Name; // pointer to temporary allocator
		RenderGraphPassId Id; // index to internal Passes array
		RenderGraphPassType Type;
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies;
		RenderGraphExecutionFunc ExecutionFunc;

		// TODO: unify textures and buffers internally?
		RenderGraphDynamicArray<RenderGraphTextureBarrier> TextureBarriers;
		RenderGraphDynamicArray<RenderGraphBufferBarrier>  BufferBarriers;

		VkRenderPass VulkanRenderPass = NULL;
		VkFramebuffer VulkanFramebuffer = NULL;
	};

	struct RenderGraphPooledTexture
	{
		SPtr<Texture2> Texture;
		bool Used = false;
		int UnusedFrames = 0; // how many frames behind it was last used
	};

	struct RenderGraphPooledBuffer
	{
		SPtr<Buffer> Buffer;
		bool Used = false;
		int UnusedFrames = 0; // how many frames behind it was last used
	};

	// valid only while render graph is valid
	struct RenderGraphDebugInformation
	{
		std::span<RenderPass> Passes;
		std::span<RenderGraphTexture> Textures;
		std::span<RenderGraphBuffer>  Buffers;
	};

	struct RenderGraphTextureExtraction
	{
		RenderGraphTextureRef Src;
		int Version;
		Texture2** Dst;
	};

	// for internal use
	struct RenderGraphMarker
	{
		enum MarkerType
		{
			TypePush,
			TypePop,
		};

		const char* Marker; // pointer to internal allocator memory
		RenderGraphPassId PassId;
		MarkerType Type;
	};

	struct RenderGraphScopedMarker
	{
		RenderGraphScopedMarker(RenderGraph& Graph, const char* Marker);
		~RenderGraphScopedMarker();

		RenderGraph& Graph;
	};

	#define RENDER_GRAPH_SCOPED_MARKER(Graph, Marker) RenderGraphScopedMarker _RG_ScopedMarker_##LINE(Graph, Marker);

	struct RenderGraphExecuteParameters
	{
		iVector2 DefaultViewportSize;  // when renderpasses don't explicitly provide viewport size, this will be used
		VkSemaphore WaitSemaphore;     // RG will start first submit after this one is signaled
		SPtr<FenceVulkan> SignalFence; // optional fence that will be signaled after RG work is finished
	};

	struct RenderGraphExecuteResults
	{
		VkSemaphore FinishSemaphore; // is signaled when all RG submits are done
	};

	struct RenderGraphHistoryInvalidation
	{
		Texture2* Dst;
	};

	class RenderGraph
	{
	public:
		RenderGraph(SPtr<DeviceVulkan> Device, SPtr<GPUScene> Scene);
		~RenderGraph();

		RenderGraphTextureRef CreateTexture(const TextureDesc2& Desc, const char* Name);
		RenderGraphBufferRef  CreateBuffer(const BufferDesc& Desc, const char* Name);

		TextureDesc2 GetTextureDesc(RenderGraphTextureRef Texture) const;
		BufferDesc   GetBufferDesc(RenderGraphBufferRef Buffer) const;
		const char* GetTextureName(RenderGraphTextureRef Texture) const;
		const char* GetBufferName(RenderGraphBufferRef Buffer) const;
		iVector2    GetTextureSize2D(RenderGraphTextureRef Texture) const;

		// use only after execution has been finished
		// texture is valid only before the next Clear
		Texture2* GetTextureAfterExecution(RenderGraphTextureRef Texture) const;

		void AddPass(const char* Name, RenderGraphPassType Type, RenderPassParameters Parameters, RenderPassDependencies Dependencies, RenderGraphExecutionFunc ExecuteCallback);

		// Use with scoped marker
		void PushMarker(const char* Marker);
		void PopMarker();

		// copies Src graph texture into Dst after execution
		// creates a new texture if doesn't exist
		void ExtractTexture(RenderGraphTextureRef Src, Texture2** Dst);

		// Creates a new *Dst if texture is invalid
		// if invalidated, memsets texture to all zeroes before executing the render graph
		// returns true on invalidation
		bool CreateHistoryTexture(Texture2** Dst, const TextureDesc2& Desc, const char* DebugName);

		void Clear();
		RenderGraphExecuteResults Execute(const RenderGraphExecuteParameters& Parameters);

		// These functions should be used after graph was built
		std::string ExportGraphviz(); // exports graphviz dot format
		void PrintDebugInformation();
		void GetDebugInformation(RenderGraphDebugInformation& Info);

	private:
		void Build();

		void AllocateTexture(RenderGraphTexture& Texture);
		void AllocateBuffer(RenderGraphBuffer& Buffer);

		VkRenderPass GetOrCreateVulkanRenderPass(RenderGraphPassParametersRHI& AttachmentParams);
		VkFramebuffer GetOrCreateVulkanFramebuffer(RenderGraphPassParametersRHI& AttachmentParams, VkRenderPass RenderPassVulkan);

	public:
		// must be initialised first (and destroyed last)
		RenderGraphAllocator Allocator{ RenderGraphMemoryPoolSize };
	private:
		std::vector<RenderPass> Passes;
		std::vector<RenderGraphTexture> Textures; // textures that are used in the current graph
		std::vector<RenderGraphBuffer> Buffers;   // buffers that are used in the current graph
		std::vector<RenderGraphTextureExtraction> Extractions;
		std::queue<RenderGraphMarker> MarkersStack;
		std::vector<RenderGraphHistoryInvalidation> TextureInvalidations;

		std::vector<std::pair<RenderGraphPassParametersRHI, VkRenderPass>> VulkanRenderPasses;
		std::vector<std::pair<RenderGraphPassParametersRHI, VkFramebuffer>> VulkanFramebuffers;

		using TexturePool = std::vector<RenderGraphPooledTexture>;
		using BufferPool = std::vector<RenderGraphPooledBuffer>;

		std::unordered_map<TextureDesc2, TexturePool, HashTextureDesc2> TextureResourcePool;
		std::unordered_map<BufferDesc, BufferPool, HashBufferDesc> BufferResourcePool;

		bool ExecutionHasFinished = false;
	public:
		SPtr<DeviceVulkan> Device;
		SPtr<GPUScene> Scene;

		RenderGraphData RenderData;

	private:
		friend RenderGraphContext;
	};

}
