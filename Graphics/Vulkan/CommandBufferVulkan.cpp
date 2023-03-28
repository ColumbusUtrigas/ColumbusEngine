#include "Graphics/GraphicsPipeline.h"
#include <Graphics/Vulkan/CommandBufferVulkan.h>
#include <Graphics/Vulkan/GraphicsPipelineVulkan.h>
#include <Core/Assert.h>
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	void CommandBufferVulkan::Reset()
	{
		vkResetCommandBuffer(_CmdBuf, 0);
	}

	void CommandBufferVulkan::Begin()
	{
		VkCommandBufferBeginInfo info;
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.pInheritanceInfo = nullptr; // TODO: secondary buffers

		if (vkBeginCommandBuffer(_CmdBuf, &info) != VK_SUCCESS)
		{
			COLUMBUS_ASSERT_MESSAGE(false, "Failed to begin Vulkan command buffer");
		}
	}

	void CommandBufferVulkan::End()
	{
		if (vkEndCommandBuffer(_CmdBuf) != VK_SUCCESS)
		{
			COLUMBUS_ASSERT_MESSAGE(false, "Failed to end Vulkan command buffer");
		}
	}

	void CommandBufferVulkan::BeginRenderPass(VkRenderPass renderPass, VkRect2D renderArea, VkFramebuffer framebuffer, uint32_t clearValuesCount, VkClearValue* clearValues)
	{
		VkRenderPassBeginInfo info;
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.pNext = nullptr;
		info.renderPass = renderPass;
		info.renderArea = renderArea;
		info.framebuffer = framebuffer;
		info.clearValueCount = clearValuesCount;
		info.pClearValues = clearValues;

		vkCmdBeginRenderPass(_CmdBuf, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBufferVulkan::EndRenderPass()
	{
		vkCmdEndRenderPass(_CmdBuf);
	}

	void CommandBufferVulkan::BindDescriptorSet(VkDescriptorSet Set, VkPipelineLayout Layout)
	{
		vkCmdBindDescriptorSets(_CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE,
			Layout, 0, 1, &Set, 0, nullptr);
	}

	void CommandBufferVulkan::BindComputePipeline(VkPipeline Pipeline)
	{
		vkCmdBindPipeline(_CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
	}

	void CommandBufferVulkan::BindGraphicsPipeline(const Graphics::GraphicsPipeline* Pipeline)
	{
		vkCmdBindPipeline(_CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<const Graphics::GraphicsPipelineVulkan*>(Pipeline)->pipeline);
	}

	void CommandBufferVulkan::PushConstants(VkPipelineLayout layout, uint32_t offset, uint32_t size, const void* pValues)
	{
		vkCmdPushConstants(_CmdBuf, layout, VK_SHADER_STAGE_ALL, offset, size, pValues);
	}

	void CommandBufferVulkan::BindDescriptorSets(const Graphics::GraphicsPipeline* pipeline, uint32 firstSet, uint32 setCount, const VkDescriptorSet* sets)
	{
		auto vkpipe = static_cast<const Graphics::GraphicsPipelineVulkan*>(pipeline);
		
		vkCmdBindDescriptorSets(_CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, vkpipe->layout, firstSet, setCount, sets, 0, nullptr);
	}

	void CommandBufferVulkan::BindVertexBuffers(uint32_t first, uint32_t count, const BufferVulkan* buffers, const VkDeviceSize* offsets)
	{
		VkBuffer bufs[128];
		for (uint32_t i = 0; i < count; i++)
			bufs[i] = buffers[i].Buffer;

		vkCmdBindVertexBuffers(_CmdBuf, first, count, bufs, offsets);
	}

	void CommandBufferVulkan::BindIndexBuffer(const BufferVulkan buffer, const VkDeviceSize offset)
	{
		vkCmdBindIndexBuffer(_CmdBuf, buffer.Buffer, offset, VK_INDEX_TYPE_UINT32);
	}

	void CommandBufferVulkan::Dispatch(uint32 X, uint32 Y, uint32 Z)
	{
		vkCmdDispatch(_CmdBuf, X, Y, Z);
	}

	void CommandBufferVulkan::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
	{
		vkCmdDraw(_CmdBuf, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandBufferVulkan::DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset, uint32 firstInstance)
	{
		vkCmdDrawIndexed(_CmdBuf, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

}
