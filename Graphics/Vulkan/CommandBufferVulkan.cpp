#include <Graphics/Vulkan/CommandBufferVulkan.h>

#include "Common.h"
#include "BufferVulkan.h"
#include "ComputePipelineVulkan.h"
#include "Graphics/Vulkan/TypeConversions.h"
#include "GraphicsPipelineVulkan.h"
#include "RayTracingPipelineVulkan.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Columbus
{

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

	CommandBufferVulkan::~CommandBufferVulkan()
	{
		vkFreeCommandBuffers(_Device, _Pool, 1, &_CmdBuf);
	}

}
