#pragma once

#include <Graphics/Core/GraphicsCore.h>
#include <Lib/VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Columbus
{

	class TextureVulkan : public Texture2
	{
	public:
		VkImageLayout _Layout;
		VkImage _Image;
		VkImageView _View; // colour or depth-stencil view
		VkImageView _DepthView = NULL; // TODO?
		VkImageView _StencilView = NULL; // TODO?
		VkSampler _Sampler;
		VmaAllocation _Allocation;
	public:
		TextureVulkan(const TextureDesc2& Desc) : Texture2(Desc) {}

		void SetSize(size_t SizeBytes)
		{
			Size = SizeBytes;
		}
	};

}
