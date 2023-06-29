#pragma once

#include "Core/fixed_vector.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Vulkan/AccelerationStructureVulkan.h"
#include "Graphics/Vulkan/DeviceVulkanFunctions.h"
#include "Graphics/Vulkan/FenceVulkan.h"
#include "Graphics/Vulkan/PipelineDescriptorSetLayoutVulkan.h"
#include "Graphics/Vulkan/TypeConversions.h"
#include "Graphics/Vulkan/VulkanShaderCompiler.h"
#include "Graphics/Vulkan/DescriptorCache.h"
#include "VulkanMemoryAllocator/include/vk_mem_alloc.h"
#include <Core/Assert.h>
#include <Core/SmartPointer.h>

#include <Graphics/Vulkan/SwapchainVulkan.h>
#include <Graphics/Vulkan/CommandBufferVulkan.h>
#include <Graphics/Vulkan/BufferVulkan.h>
#include <Graphics/Vulkan/TextureVulkan.h>
#include <Core/Types.h>

#include <vulkan/vulkan.h>
#include <vector>
#include <cassert>

#include <Common/Image/Image.h>

namespace Columbus
{

	/**Represents device (GPU) on which Vulkan is executed.*/
	class DeviceVulkan
	{
	public:
		VkPhysicalDevice _PhysicalDevice;
		VkDevice _Device;

		VkPhysicalDeviceVulkan12Properties _Vulkan12Properties;
		VkPhysicalDeviceVulkan12Features _Vulkan12Features;

		VkPhysicalDeviceAccelerationStructurePropertiesKHR _AccelerationStructureProperties;
		VkPhysicalDeviceAccelerationStructureFeaturesKHR _AccelerationStructureFeatures;

		VkPhysicalDeviceRayTracingPipelinePropertiesKHR _RayTracingProperties;
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR _RayTracingFeatures;

		VkPhysicalDeviceProperties2 _DeviceProperties;
		VkPhysicalDeviceFeatures2 _DeviceFeatures;
		VkPhysicalDeviceMemoryProperties _MemoryProperties;

		uint32 _FamilyIndex; // TODO: multiple families, now supports only graphics
		SmartPointer<VkQueue> _ComputeQueue; // TODO: multiple queues

		VkCommandPool _CmdPool;
		VkDescriptorPool _DescriptorPool;

		VmaAllocator _Allocator;

		VulkanFunctions VkFunctions;
	private:
		VkPipelineLayout _CreatePipelineLayout(const std::vector<ShaderStageBuildResultVulkan>& Stages, PipelineDescriptorSetLayoutsVulkan& SetLayouts);
		VkDescriptorSet _CreateDescriptorSet(const PipelineDescriptorSetLayoutsVulkan& SetLayouts, int Index);
		void _SetDebugName(uint64_t ObjectHandle, VkObjectType Type, const char* Name);

		TextureVulkan* _CreateTexture(const TextureDesc2& Desc);
	public:
		DeviceVulkan(VkPhysicalDevice PhysicalDevice, VkInstance Instance) :
			_PhysicalDevice(PhysicalDevice)
		{
			VkFunctions.LoadFunctions(Instance);

			_AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
			_AccelerationStructureFeatures.pNext = nullptr;
			_RayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
			_RayTracingFeatures.pNext = &_AccelerationStructureFeatures;
			_Vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			_Vulkan12Features.pNext = &_RayTracingFeatures;
			_DeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			_DeviceFeatures.pNext = &_Vulkan12Features;

			vkGetPhysicalDeviceFeatures2(PhysicalDevice, &_DeviceFeatures);

			_RayTracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
			_RayTracingProperties.pNext = nullptr;
			_DeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			_DeviceProperties.pNext = &_RayTracingProperties;

			vkGetPhysicalDeviceProperties2(PhysicalDevice, &_DeviceProperties);
			vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &_MemoryProperties);

			// logic if features are enabled/disabled
			COLUMBUS_ASSERT(_Vulkan12Features.descriptorIndexing); // non-uniform descriptor indexing
			COLUMBUS_ASSERT(_Vulkan12Features.descriptorBindingPartiallyBound);
			COLUMBUS_ASSERT(_Vulkan12Features.bufferDeviceAddress);
			COLUMBUS_ASSERT(_RayTracingFeatures.rayTracingPipeline);

			Log::Message("Creating Vulkan logical device on %s", _DeviceProperties.properties.deviceName);

			// enumerate queue families
			uint32_t queueFamilyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(_PhysicalDevice, &queueFamilyCount, nullptr);
			std::vector<VkQueueFamilyProperties> qFamProps(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(_PhysicalDevice, &queueFamilyCount, qFamProps.data());

			// find first suitable queue family
			_FamilyIndex = -1;
			for (uint32 i = 0; i < qFamProps.size(); i++)
			{
				if (qFamProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					_FamilyIndex = i;
					break;
				}
			}

			COLUMBUS_ASSERT_MESSAGE(_FamilyIndex != -1, "Failed to choose queue family");

			float queuePriorities = 1.0f;
			VkDeviceQueueCreateInfo queueCreateInfo;
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = _FamilyIndex;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriorities;

			std::vector<const char*> extensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME,

				VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
				VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
				VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
				VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME
			};

			VkDeviceCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			info.pNext = &_DeviceFeatures;
			info.flags = 0;
			info.queueCreateInfoCount = 1;
			info.pQueueCreateInfos = &queueCreateInfo;
			info.enabledLayerCount = 0;
			info.ppEnabledLayerNames = nullptr;
			info.enabledExtensionCount = extensions.size();
			info.ppEnabledExtensionNames = extensions.data();
			info.pEnabledFeatures = nullptr;

			VK_CHECK(vkCreateDevice(PhysicalDevice, &info, nullptr, &_Device));

			_ComputeQueue = SmartPointer<VkQueue>(new VkQueue);
			vkGetDeviceQueue(_Device, _FamilyIndex, 0, _ComputeQueue.Get());

			// create buffer pool
			VkCommandPoolCreateInfo commandPoolInfo;
			commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolInfo.pNext = nullptr;
			commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPoolInfo.queueFamilyIndex = _FamilyIndex;

			VK_CHECK(vkCreateCommandPool(_Device, &commandPoolInfo, nullptr, &_CmdPool));

			// create descriptor pool
			VkDescriptorPoolSize poolSizes[5];
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSizes[0].descriptorCount = 100;
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = 100;
			poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			poolSizes[2].descriptorCount = 10;
			poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSizes[3].descriptorCount = 10;
			poolSizes[4].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
			poolSizes[4].descriptorCount = 10;

			VkDescriptorPoolCreateInfo descriptorPoolInfo;
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.pNext = nullptr;
			descriptorPoolInfo.flags = 0;
			descriptorPoolInfo.maxSets = 1024;
			descriptorPoolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
			descriptorPoolInfo.pPoolSizes = poolSizes;

			VK_CHECK(vkCreateDescriptorPool(_Device, &descriptorPoolInfo, nullptr, &_DescriptorPool));

			// initialize VMA
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = PhysicalDevice;
			allocatorInfo.device = _Device;
			allocatorInfo.instance = Instance;
			allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
			vmaCreateAllocator(&allocatorInfo, &_Allocator);
		}

		// Low-level API abstraction

		SwapchainVulkan* CreateSwapchain(VkSurfaceKHR surface);

		VkRenderPass CreateRenderPass(const std::vector<AttachmentDesc>& Attachments);
		VkRenderPass CreateRenderPass(VkFormat format);

		VkFramebuffer CreateFramebuffer(VkRenderPass Renderpass, const std::vector<Texture2*>& Textures);

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

		void UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const Buffer* Buffer);
		void UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const Texture2* Texture);
		void UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const AccelerationStructure* TLAS);

		// TODO: streaming
		Buffer* CreateBuffer(const BufferDesc& Desc, const void* InitialData);

		// TODO: data change, streaming, layout transitions
		Texture2* CreateTexture(const TextureDesc2& Desc);
		Texture2* CreateTexture(const Image& Image);

		// TODO: data sync, bariers

		AccelerationStructure* CreateAccelerationStructure(const AccelerationStructureDesc& Desc);

		void SetDebugName(const CommandBufferVulkan* CmdBuf, const char* Name);
		void SetDebugName(const ComputePipeline* Pipeline, const char* Name);
		void SetDebugName(const GraphicsPipeline* Pipeline, const char* Name);
		void SetDebugName(const RayTracingPipeline* Pipeline, const char* Name);
		void SetDebugName(const Buffer* Buffer, const char* Name);
		void SetDebugName(const Texture2* Texture, const char* Name);
		void SetDebugName(const AccelerationStructure* AccelerationStructure, const char* Name);

		// TODO: Higher-level API abstraction
		// GPUScene* CreateGPUScene(const char* Name);

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

			return vkGetBufferDeviceAddress(_Device, &info);
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

		void WaitForFence(SPtr<FenceVulkan> fence, uint64_t timeout)
		{
			vkWaitForFences(_Device, 1, &fence->_Fence, true, timeout);
		}

		void ResetFence(SPtr<FenceVulkan> fence)
		{
			vkResetFences(_Device, 1, &fence->_Fence);
		}

		void AcqureNextImage(SwapchainVulkan* swapchain, VkSemaphore signalSemaphore, uint32_t& imageIndex)
		{
			vkAcquireNextImageKHR(_Device, swapchain->swapChain, UINT64_MAX, signalSemaphore, nullptr, &imageIndex);
		}

		void Submit(CommandBufferVulkan* Buffer, SPtr<FenceVulkan> fence, uint32_t waitSemaphoresCount, VkSemaphore* waitSemaphores, uint32_t signalSemaphoresCount, VkSemaphore* signalSemaphores)
		{
			VkPipelineStageFlags waitStages[] = {
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
			};

			VkSubmitInfo submit_info;
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext = nullptr;
			submit_info.waitSemaphoreCount = waitSemaphoresCount;
			submit_info.pWaitSemaphores = waitSemaphores;
			submit_info.pWaitDstStageMask = waitStages;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &Buffer->_GetHandle();
			submit_info.signalSemaphoreCount = signalSemaphoresCount;
			submit_info.pSignalSemaphores = signalSemaphores;

			VK_CHECK(vkQueueSubmit(*_ComputeQueue, 1, &submit_info, fence->_Fence));
		}

		void Submit(CommandBufferVulkan* Buffer)
		{
			VkSubmitInfo submit_info;
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext = nullptr;
			submit_info.waitSemaphoreCount = 0;
			submit_info.pWaitSemaphores = nullptr;
			submit_info.pWaitDstStageMask = nullptr;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &Buffer->_CmdBuf;
			submit_info.signalSemaphoreCount = 0;
			submit_info.pSignalSemaphores = nullptr;

			VK_CHECK(vkQueueSubmit(*_ComputeQueue, 1, &submit_info, NULL));
		}

		void QueueWaitIdle()
		{
			VK_CHECK(vkQueueWaitIdle(*_ComputeQueue));
		}

		void Present(SwapchainVulkan* swapchain, uint32_t imageIndex, VkSemaphore waitSemaphore)
		{
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &waitSemaphore;

			VkSwapchainKHR swapChains[] = { swapchain->swapChain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &imageIndex;

			vkQueuePresentKHR(*_ComputeQueue, &presentInfo);
		}

		~DeviceVulkan()
		{
			// VK_CHECK(vkDeviceWaitIdle(_Device));

			// vmaDestroyAllocator(_Allocator);
			// vkDestroyDescriptorPool(_Device, _DescriptorPool, nullptr);
			// vkDestroyCommandPool(_Device, _CmdPool, nullptr);
			// vkDestroyDevice(_Device, nullptr);
		}
	};

}
