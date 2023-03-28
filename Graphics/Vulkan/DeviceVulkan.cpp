#include "Graphics/Vulkan/TypeConversions.h"
#include <Graphics/Vulkan/DeviceVulkan.h>
#include <cstring>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define VMA_IMPLEMENTATION
#include <Lib/VulkanMemoryAllocator/include/vk_mem_alloc.h>

namespace Columbus
{

	SwapchainVulkan* DeviceVulkan::CreateSwapchain(VkSurfaceKHR surface)
	{
		return new SwapchainVulkan(_Device, _PhysicalDevice, surface);
	}

	VkRenderPass DeviceVulkan::CreateRenderPass(VkFormat format)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkRenderPass renderPass;

		if (vkCreateRenderPass(_Device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan Render Pass");
		}

		return renderPass;
	}

	void DeviceVulkan::CreateFramebuffers(SwapchainVulkan* swapchain, VkRenderPass renderpass)
	{
		swapchain->swapChainFramebuffers.resize(swapchain->swapChainImageViews.size());

		for (size_t i = 0; i < swapchain->swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] =
			{
				swapchain->swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderpass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapchain->swapChainExtent.width;
			framebufferInfo.height = swapchain->swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(_Device, &framebufferInfo, nullptr, &swapchain->swapChainFramebuffers[i]) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan Framebuffer")
			}
		}
	}

	BufferVulkan CreateStagingBufferVulkanInternal(VmaAllocator allocator, size_t size, const void* data)
	{
		BufferVulkan buffer;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		vmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		vmaallocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo,
			&buffer.Buffer,
			&buffer.Allocation,
			nullptr);

		if (data != nullptr)
		{
			void* mapped;
			vmaMapMemory(allocator, buffer.Allocation, &mapped);
			memcpy(mapped, data, size);
			vmaUnmapMemory(allocator, buffer.Allocation);
		}

		return buffer;
	}

	void TransitionImageLayout(VkCommandBuffer cmdbuf, VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkAccessFlags srcAccessMaks, VkAccessFlags dstAccessMask,
		VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = srcAccessMaks;
		barrier.dstAccessMask = dstAccessMask;
		vkCmdPipelineBarrier(
			cmdbuf,
			srcStageMask, dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	BufferVulkan DeviceVulkan::CreateBuffer(size_t Size, const void* Data, BufferType type)
	{
		BufferVulkan buffer;

		auto staging = CreateStagingBufferVulkanInternal(_Allocator, Size, Data);

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = Size;
		bufferInfo.usage = BufferTypeToVK(type) | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateBuffer(_Allocator, &bufferInfo, &vmaallocInfo,
			&buffer.Buffer,
			&buffer.Allocation,
			nullptr);

		auto copyCmdBuf = CreateCommandBuffer();
		copyCmdBuf.Begin();
		VkBufferCopy copy;
		copy.srcOffset = 0;
		copy.dstOffset = 0;
		copy.size = Size;
		vkCmdCopyBuffer(copyCmdBuf._CmdBuf, staging.Buffer, buffer.Buffer, 1, &copy);
		copyCmdBuf.End();

		VkSubmitInfo submit_info;
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;
		submit_info.waitSemaphoreCount = 0;
		submit_info.pWaitSemaphores = NULL;
		submit_info.pWaitDstStageMask = 0;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &copyCmdBuf._CmdBuf;
		submit_info.signalSemaphoreCount = 0;
		submit_info.pSignalSemaphores = NULL;

		vkQueueSubmit(*_ComputeQueue, 1, &submit_info, NULL);
		vkQueueWaitIdle(*_ComputeQueue);

		vmaDestroyBuffer(_Allocator, staging.Buffer, staging.Allocation);

		return buffer;
	}

	TextureVulkan DeviceVulkan::CreateTexture(const Image& image)
	{
		TextureVulkan texture;

		auto staging = CreateStagingBufferVulkanInternal(_Allocator, image.GetSize(0), image.Get2DData(0));

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = NULL;
		imageInfo.flags = 0;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = TextureFormatToVK(image.GetFormat());
		imageInfo.extent.width = image.GetWidth();
		imageInfo.extent.height = image.GetHeight();
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 1;
		imageInfo.pQueueFamilyIndices = &_FamilyIndex;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;

		auto result = vmaCreateImage(_Allocator, &imageInfo, &vmaallocInfo,
				&texture.image,
				&texture.allocation,
				nullptr);
		printf("ERROR CODE: %i\n", result);

		auto copyCmdBuf = CreateCommandBuffer();
		copyCmdBuf.Begin();

		TransitionImageLayout(copyCmdBuf._CmdBuf,
			texture.image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
		);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = {0, 0, 0};
		region.imageExtent = { image.GetWidth(), image.GetHeight(), 1 };
		vkCmdCopyBufferToImage(copyCmdBuf._CmdBuf,
			staging.Buffer,
			texture.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		TransitionImageLayout(copyCmdBuf._CmdBuf,
			texture.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
		);

		copyCmdBuf.End();
		VkSubmitInfo submit_info;
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;
		submit_info.waitSemaphoreCount = 0;
		submit_info.pWaitSemaphores = nullptr;
		submit_info.pWaitDstStageMask = 0;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &copyCmdBuf._CmdBuf;
		submit_info.signalSemaphoreCount = 0;
		submit_info.pSignalSemaphores = nullptr;

		vkQueueSubmit(*_ComputeQueue, 1, &submit_info, NULL);
		vkQueueWaitIdle(*_ComputeQueue);

		vmaDestroyBuffer(_Allocator, staging.Buffer, staging.Allocation);

		VkImageViewCreateInfo viewInfo;
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.image = texture.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = TextureFormatToVK(image.GetFormat());
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		vkCreateImageView(_Device, &viewInfo, nullptr, &texture.view);

		VkSamplerCreateInfo samplerInfo;
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext = nullptr;
		samplerInfo.flags = 0;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.mipLodBias = 0;
		samplerInfo.anisotropyEnable = false;
		samplerInfo.maxAnisotropy = 1;
		samplerInfo.compareEnable = false;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		samplerInfo.unnormalizedCoordinates = false;
		vkCreateSampler(_Device, &samplerInfo, nullptr, &texture.sampler);

		return texture;
	}

}
