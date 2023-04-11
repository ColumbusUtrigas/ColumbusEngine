#include "CommandBufferVulkan.h"

#include "TypeConversions.h"
#include "DeviceVulkan.h"

#include "ComputePipelineVulkan.h"
#include "GraphicsPipelineVulkan.h"
#include "RayTracingPipelineVulkan.h"
#include "BufferVulkan.h"

#include <cstring>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
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

		VK_CHECK(vkCreateRenderPass(_Device, &renderPassInfo, nullptr, &renderPass));

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

			VK_CHECK(vkCreateFramebuffer(_Device, &framebufferInfo, nullptr, &swapchain->swapChainFramebuffers[i]));
		}
	}

	CommandBufferVulkan DeviceVulkan::CreateCommandBuffer()
	{
		VkCommandBufferAllocateInfo commandBufferInfo;
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.pNext = nullptr;
		commandBufferInfo.commandPool = _CmdPool;
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferInfo.commandBufferCount = 1;

		VkCommandBuffer result;
		VK_CHECK(vkAllocateCommandBuffers(_Device, &commandBufferInfo, &result));

		return CommandBufferVulkan(_Device, _CmdPool, result, VkFunctions);
	}

	VkDescriptorSet DeviceVulkan::CreateDescriptorSet(const ComputePipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const ComputePipelineVulkan*>(Pipeline);

		return _CreateDescriptorSet(vkpipe->pipeline, vkpipe->SetLayouts, Index);
	}

	VkDescriptorSet DeviceVulkan::CreateDescriptorSet(const GraphicsPipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const Graphics::GraphicsPipelineVulkan*>(Pipeline);

		return _CreateDescriptorSet(vkpipe->pipeline, vkpipe->SetLayouts, Index);
	}

	VkDescriptorSet DeviceVulkan::CreateDescriptorSet(const RayTracingPipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const RayTracingPipelineVulkan*>(Pipeline);

		return _CreateDescriptorSet(vkpipe->pipeline, vkpipe->SetLayouts, Index);
	}

	void DeviceVulkan::UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const Buffer* Buffer)
	{
		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = static_cast<const BufferVulkan*>(Buffer)->_Buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet write;
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = Set;
		write.dstBinding = BindingId;
		write.dstArrayElement = ArrayId;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write.pImageInfo = nullptr;
		write.pBufferInfo = &bufferInfo;
		write.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
	}

	void DeviceVulkan::UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const TextureVulkan* Texture)
	{
		VkDescriptorImageInfo imageInfo;
		imageInfo.imageLayout = Texture->layout;
		imageInfo.imageView = Texture->view;
		imageInfo.sampler = Texture->sampler;

		VkWriteDescriptorSet write;
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = Set;
		write.dstBinding = BindingId;
		write.dstArrayElement = ArrayId;
		write.descriptorCount = 1;
		write.descriptorType = (Texture->_Desc.Usage & TextureVulkanUsageStorage) ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageInfo;
		write.pBufferInfo = nullptr;
		write.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
	}

	void DeviceVulkan::UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const AccelerationStructure* TLAS)
	{
		VkWriteDescriptorSetAccelerationStructureKHR ASinfo{};
		ASinfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		ASinfo.accelerationStructureCount = 1;
		ASinfo.pAccelerationStructures = &static_cast<const AccelerationStructureVulkan*>(TLAS)->_Handle;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// The specialized acceleration structure descriptor has to be chained
		write.pNext = &ASinfo;
		write.dstSet = Set;
		write.dstBinding = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
	}

	// TODO: REMOVE
	struct StagingBufferVulkan
	{
		VkBuffer Buffer;
		VmaAllocation Allocation;
	};

	StagingBufferVulkan CreateStagingBufferVulkanInternal(VmaAllocator allocator, size_t size, const void* data)
	{
		StagingBufferVulkan buffer;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		vmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		vmaallocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo,
			&buffer.Buffer,
			&buffer.Allocation,
			nullptr));

		if (data != nullptr)
		{
			void* mapped;
			VK_CHECK(vmaMapMemory(allocator, buffer.Allocation, &mapped));
			memcpy(mapped, data, size);
			vmaUnmapMemory(allocator, buffer.Allocation);
		}

		return buffer;
	}

	Buffer* DeviceVulkan::CreateBuffer(const BufferDesc& Desc, const void* InitialData)
	{
		// TODO: Enhance buffer creation
		auto result = new BufferVulkan(Desc);
		StagingBufferVulkan staging;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = Desc.Size;
		bufferInfo.usage = BufferTypeToVK(Desc.BindFlags);

		if (InitialData != nullptr)
		{
			staging = CreateStagingBufferVulkanInternal(_Allocator, Desc.Size, InitialData);
			bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

		//if (deviceAddres.BindFlags)
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		}
		if (Desc.UsedInAccelerationStructure)
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
		}

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VK_CHECK(vmaCreateBuffer(_Allocator, &bufferInfo, &vmaallocInfo,
			&result->_Buffer,
			&result->_Allocation,
			nullptr));

		if (InitialData != nullptr)
		{
			auto copyCmdBuf = CreateCommandBuffer();
			copyCmdBuf.Begin();
			VkBufferCopy copy = vk::BufferCopy(0, 0, Desc.Size);
			vkCmdCopyBuffer(copyCmdBuf._CmdBuf, staging.Buffer, result->_Buffer, 1, &copy);
			copyCmdBuf.End();

			Submit(copyCmdBuf);
			QueueWaitIdle();

			vmaDestroyBuffer(_Allocator, staging.Buffer, staging.Allocation);
		}

		return result;
	}

	void TransitionImageLayout(VkCommandBuffer cmdbuf, VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkAccessFlags srcAccessMaks, VkAccessFlags dstAccessMask,
		VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, int layers)
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
		barrier.subresourceRange.layerCount = layers;
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

	TextureVulkan* DeviceVulkan::CreateTexture(const TextureVulkanDesc& Desc)
	{
		auto result = new TextureVulkan(Desc);
		result->_Device = _Device;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = NULL;
		imageInfo.flags = 0;
		imageInfo.imageType = TextureTypeToImageTypeVk(Desc.Type);
		imageInfo.format = TextureFormatToVK(Desc.Format);
		imageInfo.extent.width = Desc.Width;
		imageInfo.extent.height = Desc.Height;
		imageInfo.extent.depth = Desc.Depth;
		imageInfo.mipLevels = Desc.Mips;
		imageInfo.arrayLayers = Desc.ArrayLayers;
		imageInfo.samples = (VkSampleCountFlagBits)Desc.Samples; // TODO
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = Desc.Usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 1;
		imageInfo.pQueueFamilyIndices = &_FamilyIndex;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		VK_CHECK(vmaCreateImage(_Allocator, &imageInfo, &vmaallocInfo, &result->image, &result->allocation, nullptr));

		auto copyCmdBuf = CreateCommandBuffer();
		copyCmdBuf.Reset();
		copyCmdBuf.Begin();

		TransitionImageLayout(copyCmdBuf._CmdBuf,
			result->image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL,
			0, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
			1
		);
		result->layout = VK_IMAGE_LAYOUT_GENERAL;

		copyCmdBuf.End();
		Submit(copyCmdBuf);
		QueueWaitIdle();

		VkImageViewCreateInfo viewInfo;
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.image = result->image;
		viewInfo.viewType = TextureTypeToViewTypeVk(Desc.Type);
		viewInfo.format = TextureFormatToVK(Desc.Format);
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = Desc.Mips;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = Desc.ArrayLayers;
		VK_CHECK(vkCreateImageView(_Device, &viewInfo, nullptr, &result->view));

		return result;
	}

	SPtr<TextureVulkan> DeviceVulkan::CreateTexture(const Image& image)
	{
		auto texture = std::make_shared<TextureVulkan>(TextureVulkanDesc());
		texture->_Device = _Device;

		bool isCube = image.GetType() == Columbus::Image::Type::ImageCube;
		int size = isCube ? 6 : 1;

		StagingBufferVulkan staging;

		staging = CreateStagingBufferVulkanInternal(_Allocator, image.GetSize(0) * size, image.GetData());

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = NULL;
		imageInfo.flags = isCube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = TextureFormatToVK(image.GetFormat());
		imageInfo.extent.width = image.GetWidth();
		imageInfo.extent.height = image.GetHeight();
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = size;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 1;
		imageInfo.pQueueFamilyIndices = &_FamilyIndex;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;

		VK_CHECK(vmaCreateImage(_Allocator, &imageInfo, &vmaallocInfo,
				&texture->image,
				&texture->allocation,
				nullptr));

		auto copyCmdBuf = CreateCommandBuffer();
		copyCmdBuf.Reset();
		copyCmdBuf.Begin();

		TransitionImageLayout(copyCmdBuf._CmdBuf,
			texture->image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			size
		);

		VkBufferImageCopy regions[6];

		for (int i = 0; i < size; i++)
		{
			regions[i].bufferOffset = image.GetOffset(i, 0);
			regions[i].bufferRowLength = 0;
			regions[i].bufferImageHeight = 0;
			regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			regions[i].imageSubresource.mipLevel = 0;
			regions[i].imageSubresource.baseArrayLayer = i;
			regions[i].imageSubresource.layerCount = 1;
			regions[i].imageOffset = {0, 0, 0};
			regions[i].imageExtent = { image.GetWidth(), image.GetHeight(), 1 };
		}

		vkCmdCopyBufferToImage(copyCmdBuf._CmdBuf,
			staging.Buffer,
			texture->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			size,
			regions
		);

		TransitionImageLayout(copyCmdBuf._CmdBuf,
			texture->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			size
		);
		texture->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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

		VK_CHECK(vkQueueSubmit(*_ComputeQueue, 1, &submit_info, NULL));
		VK_CHECK(vkQueueWaitIdle(*_ComputeQueue));

		// vmaDestroyBuffer(_Allocator, staging.Buffer, staging.Allocation);

		VkImageViewCreateInfo viewInfo;
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.image = texture->image;
		viewInfo.viewType = isCube ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = TextureFormatToVK(image.GetFormat());
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = size;
		VK_CHECK(vkCreateImageView(_Device, &viewInfo, nullptr, &texture->view));

		VkSamplerCreateInfo samplerInfo;
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext = nullptr;
		samplerInfo.flags = 0;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		// samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		// samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		// samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias = 0;
		samplerInfo.anisotropyEnable = false;
		samplerInfo.maxAnisotropy = 1;
		samplerInfo.compareEnable = false;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		samplerInfo.unnormalizedCoordinates = false;
		VK_CHECK(vkCreateSampler(_Device, &samplerInfo, nullptr, &texture->sampler));

		return texture;
	}

	VkPipelineLayout DeviceVulkan::_CreatePipelineLayout(const std::vector<ShaderStageBuildResultVulkan>& Stages, PipelineDescriptorSetLayoutsVulkan& SetLayouts)
	{
		// Concat all push constants and descriptor sets
		std::vector<VkPushConstantRange> pushConstants;
		std::vector<DescriptorSetInfo> descriptorSets;

		for (auto& stage : Stages)
		{
			for (auto& pushConstant : stage.Spirv.pushConstants)
			{
				pushConstants.push_back(pushConstant);
			}

			for (auto& descriptorSet : stage.Spirv.DescriptorSets)
			{
				descriptorSets.push_back(descriptorSet);
			}
		}

		// TODO: Caching scheme
		for (int i = 0; i < descriptorSets.size(); i++)
		{
			auto& setInfo = descriptorSets[i];

			VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo;
			bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			bindingFlagsInfo.pNext = nullptr;
			bindingFlagsInfo.bindingCount = setInfo.BindingFlags.size();
			bindingFlagsInfo.pBindingFlags = setInfo.BindingFlags.data();

			VkDescriptorSetLayoutCreateInfo setLayoutInfo;
			setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			setLayoutInfo.pNext = nullptr;
			setLayoutInfo.flags = 0;
			setLayoutInfo.bindingCount = setInfo.Bindings.size();
			setLayoutInfo.pBindings = setInfo.Bindings.data();

			SetLayouts.VariableCountMax[i] = setInfo.VariableCountMax;
			if (setInfo.VariableCountMax > 0)
			{
				setLayoutInfo.pNext = &bindingFlagsInfo;
			}

			SetLayouts.UsedLayouts++;
			VK_CHECK(vkCreateDescriptorSetLayout(_Device, &setLayoutInfo, nullptr, &SetLayouts.Layouts[i]));
		}

		VkPipelineLayoutCreateInfo layoutInfo;
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.setLayoutCount = descriptorSets.size();
		layoutInfo.pSetLayouts = SetLayouts.Layouts;
		layoutInfo.pushConstantRangeCount = pushConstants.size();
		layoutInfo.pPushConstantRanges = pushConstants.data();

		VkPipelineLayout result;
		VK_CHECK(vkCreatePipelineLayout(_Device, &layoutInfo, nullptr, &result));
		return result;
	}

	VkDescriptorSet DeviceVulkan::_CreateDescriptorSet(VkPipeline Pipeline, const PipelineDescriptorSetLayoutsVulkan& SetLayouts, int Index)
	{
		// only for variable descriptor counts
		VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorSetCounts;
		descriptorSetCounts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
		descriptorSetCounts.pNext = nullptr;
		descriptorSetCounts.descriptorSetCount = 1;
		descriptorSetCounts.pDescriptorCounts = &SetLayouts.VariableCountMax[Index];

		VkDescriptorSetAllocateInfo descriptorSetInfo;
		descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetInfo.pNext = nullptr;
		descriptorSetInfo.descriptorPool = _DescriptorPool;
		descriptorSetInfo.descriptorSetCount = 1;
		descriptorSetInfo.pSetLayouts = &SetLayouts.Layouts[Index];

		if (SetLayouts.VariableCountMax[Index] > 0)
		{
			descriptorSetInfo.pNext = &descriptorSetCounts;
		}

		VkDescriptorSet result;

		VK_CHECK(vkAllocateDescriptorSets(_Device, &descriptorSetInfo, &result));

		return result;
	}

}
