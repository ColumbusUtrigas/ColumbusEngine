#pragma once

#include "Graphics/Vulkan/BufferVulkan.h"
#include <vulkan/vulkan.h>
#include <Core/Types.h>
#include <Graphics/GraphicsPipeline.h>

namespace Columbus
{

	class CommandBufferVulkan
	{
	public:
		VkCommandBuffer _CmdBuf;
	public:
		CommandBufferVulkan(VkCommandBuffer CmdBuf) :
			_CmdBuf(CmdBuf) {}

		void Reset();
		void Begin();
		void End();

		void BeginRenderPass(VkRenderPass renderPass, VkRect2D renderArea, VkFramebuffer framebuffer, uint32_t clearValuesCount, VkClearValue* clearValues);
		void EndRenderPass();

		void BindDescriptorSet(VkDescriptorSet Set, VkPipelineLayout Layout);
		void BindComputePipeline(VkPipeline Pipeline);
		void BindGraphicsPipeline(const Graphics::GraphicsPipeline* Pipeline);

		void PushConstants(const Graphics::GraphicsPipeline* pipeline, uint32_t offset, uint32_t size, const void* pValues);
		void BindDescriptorSets(const Graphics::GraphicsPipeline* pipeline, uint32 firstSet, uint32 setCount, const VkDescriptorSet* sets);

		void BindVertexBuffers(uint32_t first, uint32_t count, const BufferVulkan* buffers, const VkDeviceSize* offsets);
		void BindIndexBuffer(const BufferVulkan buffer, const VkDeviceSize offset);

		void Dispatch(uint32 X, uint32 Y, uint32 Z);
		void Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
		void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset, uint32 firstInstance);

		const VkCommandBuffer& _GetHandle() const { return _CmdBuf; }
	};

}
