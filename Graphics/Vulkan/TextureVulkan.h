#pragma once

#include <Graphics/Texture.h>
#include <Lib/VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Columbus
{

	class TextureVulkan : public Texture2
	{
	public:
		VkImageLayout _Layout;
		VkImage _Image;
		VkImageView _View;
		VkSampler _Sampler;
		VmaAllocation _Allocation;
	public:
		TextureVulkan(const TextureDesc2& Desc) : Texture2(Desc) {}
	};

}
