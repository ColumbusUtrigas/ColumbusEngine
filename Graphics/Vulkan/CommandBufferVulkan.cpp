#include <Graphics/Vulkan/CommandBufferVulkan.h>
#include <Core/Assert.h>

namespace Columbus
{

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

	void CommandBufferVulkan::BindDescriptorSet(VkDescriptorSet Set, VkPipelineLayout Layout)
	{
		vkCmdBindDescriptorSets(_CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE,
			Layout, 0, 1, &Set, 0, nullptr);
	}

	void CommandBufferVulkan::BindPipeline(VkPipeline Pipeline)
	{
		vkCmdBindPipeline(_CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
	}

	void CommandBufferVulkan::Dispatch(uint32 X, uint32 Y, uint32 Z)
	{
		vkCmdDispatch(_CmdBuf, X, Y, Z);
	}

}


