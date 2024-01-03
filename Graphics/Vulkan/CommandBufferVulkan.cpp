#include "CommandBufferVulkan.h"
#include "Common.h"
#include "Counters.h"
#include "BufferVulkan.h"
#include "TextureVulkan.h"
#include "PipelinesVulkan.h"
#include "QueryPoolVulkan.h"
#include "TypeConversions.h"

#include <vulkan/vulkan.h>

IMPLEMENT_COUNTING_PROFILING_COUNTER("Command buffers count", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CountingCounter_Vulkan_CommandBuffers, false);

namespace Columbus
{

	CommandBufferVulkan::CommandBufferVulkan(VkDevice Device, VkCommandPool Pool, VkCommandBuffer CmdBuf, VulkanFunctions& Functions) :
			_Device(Device), _Pool(Pool), _CmdBuf(CmdBuf), _Functions(Functions)
	{
		AddProfilingCount(CountingCounter_Vulkan_CommandBuffers, 1);
	}

	void CommandBufferVulkan::Reset()
	{
		VK_CHECK(vkResetCommandBuffer(_CmdBuf, 0));
	}

	void CommandBufferVulkan::Begin()
	{
		VkCommandBufferBeginInfo info;
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.pInheritanceInfo = nullptr; // TODO: secondary buffers

		VK_CHECK(vkBeginCommandBuffer(_CmdBuf, &info));
	}

	void CommandBufferVulkan::End()
	{
		VK_CHECK(vkEndCommandBuffer(_CmdBuf));
	}

	void CommandBufferVulkan::BeginDebugMarker(const char* Name)
	{
		VkDebugUtilsLabelEXT label {};
		label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label.pLabelName = Name;
		_Functions.vkCmdBeginDebugUtilsLabel(_CmdBuf, &label);
	}

	void CommandBufferVulkan::EndDebugMarker()
	{
		_Functions.vkCmdEndDebugUtilsLabel(_CmdBuf);
	}
	void CommandBufferVulkan::InsertDebugLabel(const char* Name)
	{
		VkDebugUtilsLabelEXT label {};
		label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label.pLabelName = Name;
		_Functions.vkCmdInsertDebugUtilsLabel(_CmdBuf, &label);
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

	void CommandBufferVulkan::BindComputePipeline(const ComputePipeline* Pipeline)
	{
		auto vkpipe = static_cast<const ComputePipelineVulkan*>(Pipeline);

		vkCmdBindPipeline(_CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, vkpipe->pipeline);
	}

	void CommandBufferVulkan::BindGraphicsPipeline(const GraphicsPipeline* Pipeline)
	{
		auto vkpipe = static_cast<const GraphicsPipelineVulkan*>(Pipeline);

		vkCmdBindPipeline(_CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, vkpipe->pipeline);
	}

	void CommandBufferVulkan::BindRayTracingPipeline(const RayTracingPipeline* Pipeline)
	{
		auto vkpipe = static_cast<const RayTracingPipelineVulkan*>(Pipeline);

		vkCmdBindPipeline(_CmdBuf, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, vkpipe->pipeline);
	}

	void CommandBufferVulkan::PushConstantsCompute(const ComputePipeline* pipeline, ShaderType stages, uint32_t offset, uint32_t size, const void* pValues)
	{
		COLUMBUS_ASSERT((stages | ShaderType::Compute) == ShaderType::Compute);

		auto vkpipe = static_cast<const ComputePipelineVulkan*>(pipeline);
		auto stageFlags = ShaderTypeToVk(stages);

		vkCmdPushConstants(_CmdBuf, vkpipe->layout, stageFlags, offset, size, pValues);
	}

	void CommandBufferVulkan::BindDescriptorSetsCompute(const ComputePipeline* Pipeline, uint32 First, uint32 Count, const VkDescriptorSet* Sets)
	{
		auto vkpipe = static_cast<const ComputePipelineVulkan*>(Pipeline);

		vkCmdBindDescriptorSets(_CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, vkpipe->layout, First, Count, Sets, 0, nullptr);
	}

	void CommandBufferVulkan::PushConstantsGraphics(const GraphicsPipeline* pipeline, ShaderType stages, uint32_t offset, uint32_t size, const void* pValues)
	{
		COLUMBUS_ASSERT((stages | ShaderType::AllGraphics) == ShaderType::AllGraphics);

		auto vkpipe = static_cast<const GraphicsPipelineVulkan*>(pipeline);
		auto stageFlags = ShaderTypeToVk(stages);

		vkCmdPushConstants(_CmdBuf, vkpipe->layout, stageFlags, offset, size, pValues);
	}

	void CommandBufferVulkan::BindDescriptorSetsGraphics(const GraphicsPipeline* pipeline, uint32 firstSet, uint32 setCount, const VkDescriptorSet* sets)
	{
		auto vkpipe = static_cast<const GraphicsPipelineVulkan*>(pipeline);
		
		vkCmdBindDescriptorSets(_CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, vkpipe->layout, firstSet, setCount, sets, 0, nullptr);
	}

	void CommandBufferVulkan::PushConstantsRayTracing(const RayTracingPipeline* pipeline, ShaderType stages, uint32_t offset, uint32_t size, const void* pValues)
	{
		COLUMBUS_ASSERT((stages | ShaderType::AllRayTracing) == ShaderType::AllRayTracing);

		auto vkpipe = static_cast<const RayTracingPipelineVulkan*>(pipeline);
		auto stageFlags = ShaderTypeToVk(stages);
		
		vkCmdPushConstants(_CmdBuf, vkpipe->layout, stageFlags, offset, size, pValues);
	}

	void CommandBufferVulkan::BindDescriptorSetsRayTracing(const RayTracingPipeline* pipeline, uint32 firstSet, uint32 setCount, const VkDescriptorSet* sets)
	{
		auto vkpipe = static_cast<const RayTracingPipelineVulkan*>(pipeline);

		vkCmdBindDescriptorSets(_CmdBuf, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, vkpipe->layout, firstSet, setCount, sets, 0, nullptr);
	}

	void CommandBufferVulkan::BindVertexBuffers(uint32_t first, uint32_t count, const Buffer** buffers, const VkDeviceSize* offsets)
	{
		VkBuffer bufs[128];
		for (uint32_t i = 0; i < count; i++)
			bufs[i] = static_cast<const BufferVulkan*>(buffers[i])->_Buffer;

		vkCmdBindVertexBuffers(_CmdBuf, first, count, bufs, offsets);
	}

	void CommandBufferVulkan::BindIndexBuffer(const Buffer* buffer, const VkDeviceSize offset)
	{
		vkCmdBindIndexBuffer(_CmdBuf, static_cast<const BufferVulkan*>(buffer)->_Buffer, offset, VK_INDEX_TYPE_UINT32);
	}

	void CommandBufferVulkan::SetViewport(float X, float Y, float Width, float Height, float MinDepth, float MaxDepth)
	{
		VkViewport Viewport {
			.x = X, .y = Y, .width = Width, .height = Height, .minDepth = MinDepth, .maxDepth = MaxDepth
		};
		vkCmdSetViewport(_CmdBuf, 0, 1, &Viewport);
	}

	void CommandBufferVulkan::SetScissor(i32 X, i32 Y, u32 Width, u32 Height)
	{
		VkRect2D Scissor {
			.offset = VkOffset2D { .x = X, .y = Y },
			.extent = VkExtent2D { .width = Width, .height = Height }

		};
		vkCmdSetScissor(_CmdBuf, 0, 1, &Scissor);
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

	void CommandBufferVulkan::TraceRays(const RayTracingPipeline* pipeline, uint32_t x, uint32_t y, uint32_t depth)
	{
		auto vkpipe = static_cast<const RayTracingPipelineVulkan*>(pipeline);

		_Functions.vkCmdTraceRays(_CmdBuf,
			&vkpipe->RayGenRegionSBT,
			&vkpipe->MissRegionSBT,
			&vkpipe->HitRegionSBT,
			&vkpipe->CallableRegionSBT,
			x, y, depth);
	}

	void CommandBufferVulkan::TransitionImageLayout(Texture2* Texture, VkImageLayout NewLayout)
	{
		auto vktex = static_cast<TextureVulkan*>(Texture);
		VkImageAspectFlags AspectFlags = TextureFormatToAspectMaskVk(Texture->GetDesc().Format);

		VkImageMemoryBarrier Barrier;
		Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		Barrier.pNext = nullptr;
		Barrier.srcAccessMask = VK_ACCESS_NONE;
		Barrier.dstAccessMask = VK_ACCESS_NONE;
		Barrier.oldLayout = vktex->_Layout;
		Barrier.newLayout = NewLayout;
		Barrier.srcQueueFamilyIndex = 0;
		Barrier.dstQueueFamilyIndex = 0;
		Barrier.image = vktex->_Image;
		Barrier.subresourceRange.aspectMask = AspectFlags;
		Barrier.subresourceRange.baseArrayLayer = 0; // TODO:
		Barrier.subresourceRange.levelCount = 1; // TODO:
		Barrier.subresourceRange.baseMipLevel = 0; // TODO:
		Barrier.subresourceRange.layerCount = 1; // TODO:

		vkCmdPipelineBarrier(_CmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, // memory barrier
			0, 0, // buffer memory barrier
			0, 1, &Barrier);

		vktex->_Layout = NewLayout;
	}

	void CommandBufferVulkan::CopyImage(const Texture2* Src, const Texture2* Dst, iVector3 SrcOffset, iVector3 DstOffset, iVector3 Size)
	{
		auto srcvk = static_cast<const TextureVulkan*>(Src);
		auto dstvk = static_cast<const TextureVulkan*>(Dst);

		VkImageCopy Region {
			.srcSubresource = VkImageSubresourceLayers{
				.aspectMask = TextureFormatToAspectMaskVk(Src->GetDesc().Format),
				.mipLevel = 0, // TODO:
				.baseArrayLayer = 0, // TODO:
				.layerCount = 1, // TODO:
			},
			.srcOffset = VkOffset3D{SrcOffset.X, SrcOffset.Y, SrcOffset.Z},
			.dstSubresource = VkImageSubresourceLayers{
				.aspectMask = TextureFormatToAspectMaskVk(Dst->GetDesc().Format),
				.mipLevel = 0, // TODO:
				.baseArrayLayer = 0, // TODO:
				.layerCount = 1, // TODO:
			},
			.dstOffset = VkOffset3D{DstOffset.X, DstOffset.Y, DstOffset.Z},
			.extent = VkExtent3D{(u32)Size.X, (u32)Size.Y, (u32)Size.Z}
		};

		vkCmdCopyImage(_CmdBuf, srcvk->_Image, srcvk->_Layout, dstvk->_Image, dstvk->_Layout, 1, &Region);
	}

	void CommandBufferVulkan::CopyBuffer(const Buffer* Src, const Buffer* Dst, u64 SrcOffset, u64 DstOffset, u64 Size)
	{
		auto srcvk = static_cast<const BufferVulkan*>(Src);
		auto dstvk = static_cast<const BufferVulkan*>(Dst);
		VkBufferCopy Region {
			.srcOffset = SrcOffset,
			.dstOffset = DstOffset,
			.size = Size
		};

		vkCmdCopyBuffer(_CmdBuf, srcvk->_Buffer, dstvk->_Buffer, 1, &Region);
	}

	void CommandBufferVulkan::ResetQueryPool(const QueryPool* Pool, u32 FirstQuery, u32 QueryCount)
	{
		const QueryPoolVulkan* vkPool = static_cast<const QueryPoolVulkan*>(Pool);

		vkCmdResetQueryPool(_CmdBuf, vkPool->_Pool, FirstQuery, QueryCount);
	}

	void CommandBufferVulkan::WriteTimestamp(const QueryPool* Pool, u32 Id)
	{
		const QueryPoolVulkan* vkPool = static_cast<const QueryPoolVulkan*>(Pool);

		vkCmdWriteTimestamp(_CmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, vkPool->_Pool, Id);
	}

	CommandBufferVulkan::~CommandBufferVulkan()
	{
		RemoveProfilingCount(CountingCounter_Vulkan_CommandBuffers, 1);
		vkFreeCommandBuffers(_Device, _Pool, 1, &_CmdBuf);
	}

}
