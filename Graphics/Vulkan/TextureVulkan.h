#pragma once

#include <Graphics/Core/GraphicsCore.h>
#include <Lib/VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Columbus
{

	class TextureVulkan : public Texture2
	{
	public:
		VkImageLayout _Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImage _Image = VK_NULL_HANDLE;
		VkImageView _View = VK_NULL_HANDLE; // colour or depth-stencil view
		VkImageView _DepthView = VK_NULL_HANDLE; // TODO?
		VkImageView _StencilView = VK_NULL_HANDLE; // TODO?
		VkSampler _Sampler = VK_NULL_HANDLE;
		VmaAllocation _Allocation = nullptr;
	public:
		TextureVulkan(const TextureDesc2& Desc) : Texture2(Desc) {}

		void SetSize(size_t SizeBytes)
		{
			Size = SizeBytes;
		}
	};

	class SamplerVulkan : public Sampler
	{
	public:
		VkSampler _Sampler;
	public:
		SamplerVulkan(const SamplerDesc& Desc) : Sampler(Desc) {}
	};

}
