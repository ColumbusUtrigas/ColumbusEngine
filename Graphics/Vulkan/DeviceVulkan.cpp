#include "CommandBufferVulkan.h"

#include "Common/Image/Image.h"
#include "Core/fixed_vector.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Vulkan/AccelerationStructureVulkan.h"
#include "TypeConversions.h"
#include "DeviceVulkan.h"

#include "PipelinesVulkan.h"
#include "BufferVulkan.h"
#include "TextureVulkan.h"

#include <cstring>
#include <memory>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#define VMA_IMPLEMENTATION
#include <Lib/VulkanMemoryAllocator/include/vk_mem_alloc.h>

namespace Columbus
{

	SwapchainVulkan* DeviceVulkan::CreateSwapchain(VkSurfaceKHR surface, SwapchainVulkan* OldSwapchain)
	{
		return new SwapchainVulkan(_Device, _PhysicalDevice, surface, OldSwapchain);
	}

	VkRenderPass DeviceVulkan::CreateRenderPass(const std::vector<AttachmentDesc>& Attachments)
	{
		fixed_vector<VkAttachmentDescription, 16> VkAttachmentDescs;
		fixed_vector<VkAttachmentReference, 16> VkColorAttachmentRefs;
		VkAttachmentReference VkDepthStencilAttachmentRef;
		VkAttachmentReference* pVkDepthStencilAttachmentRef = nullptr;

		VkPipelineStageFlags SrcStageMask = 0;
		VkPipelineStageFlags DstStageMask = 0;
		VkAccessFlags DstAccessMask = 0;

		for (size_t i = 0; i < Attachments.size(); i++)
		{
			VkAttachmentDescription Attachment{};
			Attachment.format = TextureFormatToVK(Attachments[i].Format);
			Attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			// Attachment.storeOp = AttachmentStoreOpToVK(Attachments[i].StoreOp);
			Attachment.loadOp = AttachmentLoadOpToVk(Attachments[i].LoadOp);
			Attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // TODO?
			Attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; /// TODO
			Attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // TODO

			if (Attachments[i].LoadOp == AttachmentLoadOp::Load)
			{
				Attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // TODO
			} else
			{
				Attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // TODO
			}
			Attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // TODO

			VkAttachmentReference Reference{};
			Reference.attachment = i;

			switch (Attachments[i].Type)
			{
			case AttachmentType::Color:
				SrcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				DstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				DstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				Reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				VkColorAttachmentRefs.push_back(Reference);
				break;
			case AttachmentType::DepthStencil:
				SrcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				DstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				DstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				Reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				VkDepthStencilAttachmentRef = Reference;
				pVkDepthStencilAttachmentRef = &VkDepthStencilAttachmentRef;
				break;
			}

			VkAttachmentDescs.push_back(Attachment);
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = VkColorAttachmentRefs.size();
		subpass.pColorAttachments = VkColorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = pVkDepthStencilAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = SrcStageMask;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = DstStageMask;
		dependency.dstAccessMask = DstAccessMask;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = VkAttachmentDescs.size();
		renderPassInfo.pAttachments = VkAttachmentDescs.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkRenderPass renderPass;

		VK_CHECK(vkCreateRenderPass(_Device, &renderPassInfo, nullptr, &renderPass));

		return renderPass;
	}

	VkRenderPass DeviceVulkan::CreateRenderPass(VkFormat format)
	{
		// TODO: remove this function, it's not needed anymore
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

	VkFramebuffer DeviceVulkan::CreateFramebuffer(VkRenderPass Renderpass, const iVector2& Size, const std::vector<Texture2*>& Textures)
	{
		fixed_vector<VkImageView, 16> AttachmentViews;
		for (Texture2* Attachment : Textures)
		{
			AttachmentViews.push_back(static_cast<TextureVulkan*>(Attachment)->_View);
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = Renderpass;
		framebufferInfo.attachmentCount = AttachmentViews.size();
		framebufferInfo.pAttachments = AttachmentViews.data();
		framebufferInfo.width = Size.X;
		framebufferInfo.height = Size.Y;
		framebufferInfo.layers = 1;

		VkFramebuffer Result;

		VK_CHECK(vkCreateFramebuffer(_Device, &framebufferInfo, nullptr, &Result));

		return Result;
	}

	CommandBufferVulkan* DeviceVulkan::CreateCommandBuffer()
	{
		VkCommandBufferAllocateInfo commandBufferInfo;
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.pNext = nullptr;
		commandBufferInfo.commandPool = _CmdPool;
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferInfo.commandBufferCount = 1;

		VkCommandBuffer result;
		VK_CHECK(vkAllocateCommandBuffers(_Device, &commandBufferInfo, &result));

		return new CommandBufferVulkan(_Device, _CmdPool, result, VkFunctions);
	}

	SPtr<CommandBufferVulkan> DeviceVulkan::CreateCommandBufferShared()
	{
		return std::shared_ptr<CommandBufferVulkan>(CreateCommandBuffer());
	}

	VkDescriptorSet DeviceVulkan::CreateDescriptorSetUnbounded(VkDescriptorSetLayout Layout, int MaxCount)
	{
		PipelineDescriptorSetLayoutsVulkan Layouts;
		Layouts.Layouts[0] = Layout;
		Layouts.VariableCountMax[0] = MaxCount;
		Layouts.UsedLayouts = 1;

		return _CreateDescriptorSet(Layouts, 0);
	}

	VkDescriptorSet DeviceVulkan::CreateDescriptorSet(const ComputePipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const ComputePipelineVulkan*>(Pipeline);

		return _CreateDescriptorSet(vkpipe->SetLayouts, Index);
	}

	VkDescriptorSet DeviceVulkan::CreateDescriptorSet(const GraphicsPipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const GraphicsPipelineVulkan*>(Pipeline);

		return _CreateDescriptorSet(vkpipe->SetLayouts, Index);
	}

	VkDescriptorSet DeviceVulkan::CreateDescriptorSet(const RayTracingPipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const RayTracingPipelineVulkan*>(Pipeline);

		return _CreateDescriptorSet(vkpipe->SetLayouts, Index);
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

	void DeviceVulkan::UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const Texture2* Texture)
	{
		auto vktex = static_cast<const TextureVulkan*>(Texture);

		VkDescriptorImageInfo imageInfo;
		imageInfo.imageLayout = vktex->_Layout;
		imageInfo.imageView = vktex->_View;
		imageInfo.sampler = vktex->_Sampler;

		VkWriteDescriptorSet write;
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = Set;
		write.dstBinding = BindingId;
		write.dstArrayElement = ArrayId;
		write.descriptorCount = 1;
		write.descriptorType = TextureUsageToVkDescriptorType(Texture->GetDesc().Usage);
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
			auto copyCmdBuf = CreateCommandBufferShared();
			copyCmdBuf->Begin();
			VkBufferCopy copy = vk::BufferCopy(0, 0, Desc.Size);
			vkCmdCopyBuffer(copyCmdBuf->_CmdBuf, staging.Buffer, result->_Buffer, 1, &copy);
			copyCmdBuf->End();

			Submit(copyCmdBuf.get());
			QueueWaitIdle();

			vmaDestroyBuffer(_Allocator, staging.Buffer, staging.Allocation);
		}

		return result;
	}

	void TransitionImageLayout(VkCommandBuffer cmdbuf, VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkAccessFlags srcAccessMaks, VkAccessFlags dstAccessMask,
		VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, int layers, TextureFormat format)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = TextureFormatToAspectMaskVk(format);
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

	Texture2* DeviceVulkan::CreateTexture(const TextureDesc2& Desc)
	{
		auto result = _CreateTexture(Desc);

		auto copyCmdBuf = CreateCommandBufferShared();
		copyCmdBuf->Reset();
		copyCmdBuf->Begin();

		VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		switch (Desc.Usage)
		{
		case TextureUsage::Sampled:
			newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		case TextureUsage::Storage:
			newLayout = VK_IMAGE_LAYOUT_GENERAL;
			break;
		case TextureUsage::RenderTargetColor:
			newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		case TextureUsage::RenderTargetDepth:
			newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			break;
		default: COLUMBUS_ASSERT(false);
		};

		TransitionImageLayout(copyCmdBuf->_CmdBuf,
				result->_Image, result->_Layout, newLayout,
			0, 0,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			1, Desc.Format
		);

		result->_Layout = newLayout;

		copyCmdBuf->End();
		Submit(copyCmdBuf.get());
		QueueWaitIdle();

		return result;
	}

	Texture2* DeviceVulkan::CreateTexture(const Image& Image)
	{
		TextureDesc2 desc;
		desc.Type = TextureType::Texture2D;
		desc.Usage = TextureUsage::Sampled;
		desc.Width = Image.GetWidth();
		desc.Height = Image.GetHeight();
		// desc.Depth = Image.GetDepth();
		desc.Depth = 1;
		desc.Mips = 1;
		desc.Samples = 1;
		desc.Format = Image.GetFormat();
		auto result = _CreateTexture(desc);

		auto size = 1;
		
		StagingBufferVulkan staging;
		staging = CreateStagingBufferVulkanInternal(_Allocator, Image.GetSize(0) * size, Image.GetData());

		auto copyCmdBuf = CreateCommandBufferShared();
		copyCmdBuf->Reset();
		copyCmdBuf->Begin();

		TransitionImageLayout(copyCmdBuf->_CmdBuf,
			result->_Image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			size, desc.Format
		);

		VkBufferImageCopy regions[6];

		for (int i = 0; i < size; i++)
		{
			regions[i].bufferOffset = Image.GetOffset(i, 0);
			regions[i].bufferRowLength = 0;
			regions[i].bufferImageHeight = 0;
			regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			regions[i].imageSubresource.mipLevel = 0;
			regions[i].imageSubresource.baseArrayLayer = i;
			regions[i].imageSubresource.layerCount = 1;
			regions[i].imageOffset = {0, 0, 0};
			regions[i].imageExtent = { Image.GetWidth(), Image.GetHeight(), 1 };
		}

		vkCmdCopyBufferToImage(copyCmdBuf->_CmdBuf,
			staging.Buffer,
			result->_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			size,
			regions
		);

		TransitionImageLayout(copyCmdBuf->_CmdBuf,
			result->_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			size, desc.Format
		);
		result->_Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		copyCmdBuf->End();
		Submit(copyCmdBuf.get());
		QueueWaitIdle();

		return result;
	}

	void DeviceVulkan::SetDebugName(const CommandBufferVulkan* CmdBuf, const char* Name)
	{
		_SetDebugName((uint64_t)CmdBuf->_CmdBuf, VK_OBJECT_TYPE_COMMAND_BUFFER, Name);
	}
	void DeviceVulkan::SetDebugName(const ComputePipeline* Pipeline, const char* Name)
	{
		_SetDebugName((uint64_t)static_cast<const ComputePipelineVulkan*>(Pipeline)->pipeline, VK_OBJECT_TYPE_PIPELINE, Name);
	}
	void DeviceVulkan::SetDebugName(const GraphicsPipeline* Pipeline, const char* Name)
	{
		_SetDebugName((uint64_t)static_cast<const GraphicsPipelineVulkan*>(Pipeline)->pipeline, VK_OBJECT_TYPE_PIPELINE, Name);
	}
	void DeviceVulkan::SetDebugName(const RayTracingPipeline* Pipeline, const char* Name)
	{
		_SetDebugName((uint64_t)static_cast<const RayTracingPipelineVulkan*>(Pipeline)->pipeline, VK_OBJECT_TYPE_PIPELINE, Name);
	}
	void DeviceVulkan::SetDebugName(const Buffer* Buffer, const char* Name)
	{
		_SetDebugName((uint64_t)static_cast<const BufferVulkan*>(Buffer)->_Buffer, VK_OBJECT_TYPE_BUFFER, Name);
	}
	void DeviceVulkan::SetDebugName(const Texture2* Texture, const char* Name)
	{
		auto vktex = static_cast<const TextureVulkan*>(Texture);
		_SetDebugName((uint64_t)vktex->_Image, VK_OBJECT_TYPE_IMAGE, Name);
		_SetDebugName((uint64_t)vktex->_View, VK_OBJECT_TYPE_IMAGE_VIEW, Name);
		_SetDebugName((uint64_t)vktex->_Sampler, VK_OBJECT_TYPE_SAMPLER, Name);
	}
	void DeviceVulkan::SetDebugName(const AccelerationStructure* AccelerationStructure, const char* Name)
	{
		_SetDebugName((uint64_t)static_cast<const AccelerationStructureVulkan*>(AccelerationStructure)->_Handle, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR, Name);
	}

	VkPipelineLayout DeviceVulkan::_CreatePipelineLayout(const CompiledShaderData& Bytecode, PipelineDescriptorSetLayoutsVulkan& OutSetLayouts)
	{
		// TODO: Caching scheme
		for (int i = 0; i < Bytecode.Reflection->DescriptorSets.size(); i++)
		{
			auto& setInfo = Bytecode.Reflection->DescriptorSets[i];

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

			OutSetLayouts.VariableCountMax[i] = setInfo.VariableCountMax;
			if (setInfo.VariableCountMax > 0)
			{
				setLayoutInfo.pNext = &bindingFlagsInfo;
			}

			OutSetLayouts.UsedLayouts++;
			VK_CHECK(vkCreateDescriptorSetLayout(_Device, &setLayoutInfo, nullptr, &OutSetLayouts.Layouts[i]));
		}

		VkPipelineLayoutCreateInfo layoutInfo;
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.setLayoutCount = Bytecode.Reflection->DescriptorSets.size();
		layoutInfo.pSetLayouts = OutSetLayouts.Layouts;
		layoutInfo.pushConstantRangeCount = Bytecode.Reflection->PushConstants.size();
		layoutInfo.pPushConstantRanges = Bytecode.Reflection->PushConstants.data();

		VkPipelineLayout Layout;
		VK_CHECK(vkCreatePipelineLayout(_Device, &layoutInfo, nullptr, &Layout));

		return Layout;
	}

	VkDescriptorSet DeviceVulkan::_CreateDescriptorSet(const PipelineDescriptorSetLayoutsVulkan& SetLayouts, int Index)
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

	void DeviceVulkan::_SetDebugName(uint64_t ObjectHandle, VkObjectType Type, const char* Name)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo;
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.pNext = nullptr;
		nameInfo.objectType = Type;
		nameInfo.objectHandle = ObjectHandle;
		nameInfo.pObjectName = Name;
		VK_CHECK(VkFunctions.vkSetDebugUtilsObjectName(_Device, &nameInfo));
	}

	TextureVulkan* DeviceVulkan::_CreateTexture(const TextureDesc2& Desc)
	{
		auto result = new TextureVulkan(Desc);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = NULL;
		imageInfo.flags = TextureTypeToImageFlagsVk(Desc.Type);
		imageInfo.imageType = TextureTypeToImageTypeVk(Desc.Type);
		imageInfo.format = TextureFormatToVK(Desc.Format);
		imageInfo.extent.width = Desc.Width;
		imageInfo.extent.height = Desc.Height;
		imageInfo.extent.depth = Desc.Depth;
		imageInfo.mipLevels = Desc.Mips;
		imageInfo.arrayLayers = Desc.ArrayLayers;
		imageInfo.samples = SampleCountToVk(Desc.Samples);
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = TextureUsageToImageUsageVk(Desc.Usage) | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // TODO
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 1;
		imageInfo.pQueueFamilyIndices = &_FamilyIndex;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		VK_CHECK(vmaCreateImage(_Allocator, &imageInfo, &vmaallocInfo, &result->_Image, &result->_Allocation, nullptr));

		result->_Layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImageViewCreateInfo viewInfo;
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.image = result->_Image;
		viewInfo.viewType = TextureTypeToViewTypeVk(Desc.Type);
		viewInfo.format = TextureFormatToVK(Desc.Format);
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
		viewInfo.subresourceRange.aspectMask = TextureFormatToAspectMaskVk(Desc.Format);
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = Desc.Mips;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = Desc.ArrayLayers;
		VK_CHECK(vkCreateImageView(_Device, &viewInfo, nullptr, &result->_View));

		VkSamplerCreateInfo samplerInfo;
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext = nullptr;
		samplerInfo.flags = 0;
		samplerInfo.magFilter = TextureFilterToVk(Desc.MagFilter);
		samplerInfo.minFilter = TextureFilterToVk(Desc.MinFilter);
		samplerInfo.mipmapMode = TextureFilterMipToVk(Desc.MipFilter);
		samplerInfo.addressModeU = TextureAddressModeToVk(Desc.AddressU);
		samplerInfo.addressModeV = TextureAddressModeToVk(Desc.AddressV);
		samplerInfo.addressModeW = TextureAddressModeToVk(Desc.AddressW);
		samplerInfo.mipLodBias = 0;
		samplerInfo.anisotropyEnable = Desc.Anisotropy > 1;
		samplerInfo.maxAnisotropy = Desc.Anisotropy;
		samplerInfo.compareEnable = false; // TODO
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // TODO
		samplerInfo.minLod = Desc.MinLOD;
		samplerInfo.maxLod = Desc.MaxLOD;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		samplerInfo.unnormalizedCoordinates = false;
		VK_CHECK(vkCreateSampler(_Device, &samplerInfo, nullptr, &result->_Sampler));

		return result;
	}

}
