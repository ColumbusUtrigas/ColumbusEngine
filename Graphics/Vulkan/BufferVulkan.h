#pragma once

#include <VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	struct BufferVulkan
	{
		VkBuffer Buffer;
		VmaAllocation Allocation;
	};

}
