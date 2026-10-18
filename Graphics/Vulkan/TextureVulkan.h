#pragma once

#include <Graphics/Core/Texture.h>
#include <Lib/VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace Columbus
{

	class TextureViewVulkan;

	class TextureVulkan : public Texture2
	{
	public:
		VkImageLayout _Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImage       _Image  = VK_NULL_HANDLE;
		VmaAllocation _Allocation = nullptr;
		std::vector<TextureViewVulkan*> _CachedViews;

		TextureVulkan(const TextureDesc2& Desc) : Texture2(Desc) {}

		VkImageLayout GetLayout() const { return _Layout; }
		void SetSize(size_t SizeBytes) { Size = SizeBytes; }
	};

	class SamplerVulkan : public Sampler
	{
	public:
		VkSampler _Sampler = VK_NULL_HANDLE;

		SamplerVulkan(const SamplerDesc& Desc) : Sampler(Desc) {}
	};

	class TextureViewVulkan : public TextureView
	{
	public:
		VkImageView _View = VK_NULL_HANDLE;

		TextureViewVulkan(Texture2* Texture, const TextureViewDesc& Desc) : TextureView(Texture, Desc) {}
	};


}
