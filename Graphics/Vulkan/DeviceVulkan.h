#pragma once

#include "Core/fixed_vector.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Core/Types.h"
#include "Graphics/Vulkan/AccelerationStructureVulkan.h"
#include "Graphics/Vulkan/DeviceVulkanFunctions.h"
#include "Graphics/Vulkan/FenceVulkan.h"
#include "Graphics/Vulkan/PipelineDescriptorSetLayoutVulkan.h"
#include "Graphics/Vulkan/TypeConversions.h"
#include "Graphics/Vulkan/VulkanShaderCompiler.h"
#include "Graphics/Vulkan/DescriptorCache.h"
#include "Lib/VulkanMemoryAllocator/include/vk_mem_alloc.h"
#include "Profiling/Profiling.h"
#include <Core/Assert.h>
#include <Core/SmartPointer.h>

#include <Graphics/Vulkan/SwapchainVulkan.h>
#include <Graphics/Vulkan/CommandBufferVulkan.h>
#include <Graphics/Vulkan/BufferVulkan.h>
#include <Graphics/Vulkan/TextureVulkan.h>
#include <Core/Types.h>

#include <ShaderBytecode/ShaderBytecode.h>

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <cassert>

#include <Common/Image/Image.h>

// disable to make RenderDoc captures
#define ENABLE_RAY_TRACING 1

constexpr int MaxFramesInFlight = 3;

namespace Columbus
{

	class DeviceVulkan;
	struct ProfileMarkerGPU;
	struct ProfileMarkerScopedGPU;

	class GPUProfilerVulkan
	{
	public:
		static constexpr int TimestampQueryCount = 8192;
		QueryPool* Pools[MaxFramesInFlight];

		DeviceVulkan* Device = nullptr;

		std::atomic<int> CurrentTimestampQuery[MaxFramesInFlight]{0};
		std::atomic<int> CurrentMeasurement[MaxFramesInFlight]{0};
		int CurrentFrame = 0;

		bool WasReset = false; // profiler must be reset only once per frame

		struct Measurement
		{
			ProfileCounterGPU* Counter;
			int StartTimestampId;
			int EndTimestampId;
		};

		Measurement Measurements[MaxFramesInFlight][TimestampQueryCount];

	public:
		void Init();
		void Shutdown();

		void BeginFrame();
		void EndFrame();

		void Reset(CommandBufferVulkan* CommandBuffer);
		void BeginProfileCounter(ProfileMarkerGPU& Scoped, CommandBufferVulkan* CommandBuffer);
		void EndProfileConter(ProfileMarkerGPU& Scoped, CommandBufferVulkan* CommandBuffer);
	};

	struct ProfileMarkerGPU
	{
		int Id; // measurement id
		ProfileCounterGPU* Counter;

		ProfileMarkerGPU(ProfileCounterGPU* Counter) : Counter(Counter) {}
	};

	struct ProfileMarkerScopedGPU : public ProfileMarkerGPU
	{
		GPUProfilerVulkan* Profiler;
		CommandBufferVulkan* CommandBuffer;

		ProfileMarkerScopedGPU(ProfileCounterGPU* Counter, GPUProfilerVulkan* Profiler, CommandBufferVulkan* CommandBuffer);
		~ProfileMarkerScopedGPU();
	};

	#define PROFILE_GPU(counter, profiler, commandbuffer) Columbus::ProfileMarkerScopedGPU MarkerGPU ## __LINE__ (&counter, profiler, commandbuffer);

	template <typename T>
	struct ResourceDeferredDestroyVulkan
	{
		T Resource;
		int FramesLasted = 0;
	};

	struct CBufferPoolVulkan
	{
		struct FrameCBuffer
		{
			Buffer* Buf = nullptr;
			u32 Size = 0;
			bool Used = false;
		};

		std::vector<FrameCBuffer> Pool[MaxFramesInFlight];
		int CurrentFrame = 0;

	public:
		void BeginFrame();
	};

	// a general system to upload data from Host to Device
	struct HUploadBufferRing
	{
		struct UploadEntry
		{
			Buffer* Dst;

			u32 HostOffset;
			u32 DstOffset;
			u32 Size;
		};

		static constexpr u32 DefaultUploadSize = 16 * 1024 * 1024; // 16MB
		static constexpr u32 DefaultGrowthFactor = 2;

	public:

		// schedule upload of the data into device-local Buf
		void UploadBuffer(const void* Data, u32 Size, u32 DstOffset, Buffer* Buf);

		// schedule upload but fill the data manually
		void* UploadBufferMap(u32 Size, u32 DstOffset, Buffer* Buf);

		void FlushUploads();

	private:
		friend class DeviceVulkan;
		void BeginFrame();

		void Init();
		void Realloc(int BufNum, u32 NewSize);
		void Shutdown();

	private:
		int CurrentFrame = 0;
		DeviceVulkan* Device;

		u32                      UploadOffsets   [MaxFramesInFlight]{ 0 };
		CommandBufferVulkan*     CommandBufs     [MaxFramesInFlight]{ nullptr };
		Buffer*                  UploadBuffers   [MaxFramesInFlight]{ nullptr };
		u8*                      MappedBuffers   [MaxFramesInFlight]{ nullptr };
		std::vector<UploadEntry> ScheduledUploads[MaxFramesInFlight];
		SPtr<FenceVulkan>        UploadFences    [MaxFramesInFlight];
	};

	struct DeviceDefaultTextures
	{
		Texture2* White = nullptr;
		Texture2* Black = nullptr;
		Texture2* Transparent = nullptr;
	};

	/** Represents device (GPU) on which Vulkan is executed. */
	class DeviceVulkan
	{
	public:
		VkPhysicalDevice _PhysicalDevice;
		VkDevice _Device;
		VkInstance _Instance;

		VkPhysicalDeviceVulkan12Properties _Vulkan12Properties;
		VkPhysicalDeviceVulkan12Features _Vulkan12Features;

		VkPhysicalDeviceAccelerationStructurePropertiesKHR _AccelerationStructureProperties;
		VkPhysicalDeviceAccelerationStructureFeaturesKHR _AccelerationStructureFeatures;

		VkPhysicalDeviceRayTracingPipelinePropertiesKHR _RayTracingProperties;
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR _RayTracingFeatures;
		VkPhysicalDeviceRayQueryFeaturesKHR _RayQueryFeatures;

		VkPhysicalDeviceProperties2 _DeviceProperties;
		VkPhysicalDeviceFeatures2 _DeviceFeatures;
		VkPhysicalDeviceMemoryProperties _MemoryProperties;

		uint32 _FamilyIndex; // TODO: multiple families, now supports only graphics
		SmartPointer<VkQueue> _ComputeQueue; // TODO: multiple queues

		VkCommandPool _CmdPool;
		VkDescriptorPool _DescriptorPool;

		VmaAllocator _Allocator;

		VulkanFunctions VkFunctions;

		DeviceDefaultTextures DefaultTextures;
		HUploadBufferRing UploadRing;

		GPUProfilerVulkan _Profiler;
		CBufferPoolVulkan _CBufPool;

		std::unordered_map<SamplerDesc, Sampler*, HashSamplerDesc> StaticSamplers;

		std::vector<ResourceDeferredDestroyVulkan<Buffer*>>       BufferDeferredDestroys;
		std::vector<ResourceDeferredDestroyVulkan<Texture2*>>     TextureDeferredDestroys;
		std::vector<ResourceDeferredDestroyVulkan<VkRenderPass>>  RenderPassDeferredDestroys;
		std::vector<ResourceDeferredDestroyVulkan<VkFramebuffer>> FramebufferDeferredDestroys;
	private:
		VkPipelineLayout _CreatePipelineLayout(const CompiledShaderData& Bytecode, PipelineDescriptorSetLayoutsVulkan& OutSetLayouts);
		VkDescriptorSet _CreateDescriptorSet(const PipelineDescriptorSetLayoutsVulkan& SetLayouts, int Index);
		void _SetDebugName(uint64_t ObjectHandle, VkObjectType Type, const char* Name);

		TextureVulkan* _CreateTexture(const TextureDesc2& Desc);
	public:
		DeviceVulkan(VkPhysicalDevice PhysicalDevice, VkInstance Instance);
		~DeviceVulkan();

		// Features
		bool SupportsRayTracing() const;
		bool SupportsRayQuery() const;

		// Low-level API abstraction

		SwapchainVulkan* CreateSwapchain(VkSurfaceKHR surface, SwapchainVulkan* OldSwapchain);
		VkRenderPass CreateRenderPass(std::span<AttachmentDesc> Attachments);
		VkFramebuffer CreateFramebuffer(VkRenderPass Renderpass, const iVector2& Size, std::span<Texture2*> Textures);

		void DestroyRenderPassDeferred(VkRenderPass Renderpass);
		void DestroyFramebufferDeferred(VkFramebuffer Framebuffer);

		CommandBufferVulkan* CreateCommandBuffer();
		SPtr<CommandBufferVulkan> CreateCommandBufferShared();

		// TODO: mesh shaders
		ComputePipeline* CreateComputePipeline(const ComputePipelineDesc& Desc);
		GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineDesc& Desc, VkRenderPass RenderPass);
		RayTracingPipeline* CreateRayTracingPipeline(const RayTracingPipelineDesc& Desc);

		VkDescriptorSet CreateDescriptorSetUnbounded(VkDescriptorSetLayout Layout, int MaxCount);
		VkDescriptorSet CreateDescriptorSet(const ComputePipeline* Pipeline, int Index);
		VkDescriptorSet CreateDescriptorSet(const GraphicsPipeline* Pipeline, int Index);
		VkDescriptorSet CreateDescriptorSet(const RayTracingPipeline* Pipeline, int Index);

		// TODO: refactor binding system
		void UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const Buffer* Buffer);
		void UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const Texture2* Texture, TextureBindingFlags Flags = TextureBindingFlags::AspectColour, VkDescriptorType DescriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM);
		void UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const AccelerationStructure* TLAS);
		void UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const Sampler* Sam);

		// TODO: streaming
		Buffer* CreateBuffer(const BufferDesc& Desc, const void* InitialData);
		void    DestroyBuffer(Buffer* Buf);
		void    DestroyBufferDeferred(Buffer* Buf);
		void*   MapBuffer(const Buffer* Buf);
		void    UnmapBuffer(const Buffer* Buf);

		// will return a managed filled constant buffer
		Buffer* GetConstantBufferPrepared(u32 Size, void* Data);

		// TODO: data change, streaming, layout transitions
		Texture2* CreateTexture(const TextureDesc2& Desc);
		Texture2* CreateTexture(const Image& Image);
		void      UploadTextureMipData(Texture2* Tex, int Mip, int Layer, const void* Data);
		void      DestroyTexture(Texture2* Tex);
		void      DestroyTextureDeferred(Texture2* Tex);

		Sampler* CreateSampler(const SamplerDesc& Desc);
		void     DestroySampler(Sampler* Sam);

		// creates a sampler only once
		Sampler* GetStaticSampler(const SamplerDesc& Desc);

		// helper to create a static sampler with template args
		template <
			TextureFilter2 Filter=TextureFilter2::Linear,
			TextureAddressMode Address = TextureAddressMode::ClampToEdge>
		Sampler* GetStaticSampler()
		{
			SamplerDesc Desc;
			Desc.AddressU  = Address;
			Desc.AddressV  = Address;
			Desc.AddressW  = Address;
			Desc.MagFilter = Filter;
			Desc.MinFilter = Filter;
			Desc.MipFilter = Filter;
			return GetStaticSampler(Desc);
		}

		// TODO: data sync, bariers, implement in command buffer

		AccelerationStructure* CreateAccelerationStructure(const AccelerationStructureDesc& Desc);
		void                   DestroyAccelerationStructure(AccelerationStructure* AS);
		void                   UpdateAccelerationStructureBuffer(AccelerationStructure* AS, CommandBufferVulkan* CmdBuf, u32 NumPrimitives);

		void SetDebugName(const CommandBufferVulkan* CmdBuf, const char* Name);
		void SetDebugName(const ComputePipeline* Pipeline, const char* Name);
		void SetDebugName(const GraphicsPipeline* Pipeline, const char* Name);
		void SetDebugName(const RayTracingPipeline* Pipeline, const char* Name);
		void SetDebugName(const Buffer* Buffer, const char* Name);
		void SetDebugName(const Texture2* Texture, const char* Name);
		void SetDebugName(const AccelerationStructure* AccelerationStructure, const char* Name);

		QueryPool* CreateQueryPool(const QueryPoolDesc& Desc);
		void       DestroyQueryPool(QueryPool* Pool);
		// void       ResetQueryPool(const QueryPool* Pool, u32 FirstQuery, u32 QueryCount);
		// that call is blocking
		void       ReadQueryPoolTimestamps(const QueryPool* Pool, u32 FirstQuery, u32 QueryCount, u64* Data, u32 DataSize);

		// Higher-level API abstraction
		// GPUScene* CreateGPUScene(const char* Name);

		void BeginFrame();
		void EndFrame();

		uint32_t alignedSize(uint32_t value, uint32_t alignment)
        {
	        return (value + alignment - 1) & ~(alignment - 1);
        }

		uint32_t getHandleSizeAligned()
		{
			return alignedSize(_RayTracingProperties.shaderGroupHandleSize, _RayTracingProperties.shaderGroupHandleAlignment);
		}

		uint64_t GetBufferDeviceAddress(const Buffer* Buffer)
		{
			VkBufferDeviceAddressInfo info;
			info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			info.pNext = nullptr;
			info.buffer = static_cast<const BufferVulkan*>(Buffer)->_Buffer;

			u64 address = vkGetBufferDeviceAddress(_Device, &info);
			u64 align = Buffer->GetDesc().Alignment;
			if (align == 0)
				return address;

			return (address + (align - 1)) & ~(align - 1);
		}

		SPtr<FenceVulkan> CreateFence(bool signaled)
		{
			return std::make_shared<FenceVulkan>(_Device, signaled);
		}

		VkSemaphore CreateSemaphore()
		{
			VkSemaphoreCreateInfo semaphoreInfo;
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.pNext = nullptr;
			semaphoreInfo.flags = 0;

			VkSemaphore semaphore;
			if (vkCreateSemaphore(_Device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan Semaphore")
			}

			return semaphore;
		}

		void WaitForFence(SPtr<FenceVulkan> fence, uint64_t timeout);
		void ResetFence(SPtr<FenceVulkan> fence);

		bool AcqureNextImage(SwapchainVulkan* swapchain, VkSemaphore signalSemaphore, uint32_t& imageIndex);

		void Submit(CommandBufferVulkan* Buffer, SPtr<FenceVulkan> fence, uint32_t waitSemaphoresCount, VkSemaphore* waitSemaphores, uint32_t signalSemaphoresCount, VkSemaphore* signalSemaphores);
		void Submit(CommandBufferVulkan* Buffer);
		void QueueWaitIdle();

		void Present(SwapchainVulkan* swapchain, uint32_t imageIndex, VkSemaphore waitSemaphore);
	};

}
