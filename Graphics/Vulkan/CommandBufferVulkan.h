#pragma once

#include <Graphics/ComputePipeline.h>
#include <Graphics/GraphicsPipeline.h>
#include <Graphics/RayTracingPipeline.h>
#include <Core/Types.h>

#include "BufferVulkan.h"
#include "DeviceVulkanFunctions.h"
#include <vulkan/vulkan.h>

namespace Columbus
{

	class CommandBufferVulkan
	{
	public:
		VkDevice _Device;
		VkCommandPool _Pool;
		VkCommandBuffer _CmdBuf;

		VulkanFunctions& _Functions;
	public:
		CommandBufferVulkan(VkDevice Device, VkCommandPool Pool, VkCommandBuffer CmdBuf, VulkanFunctions& Functions) :
			_Device(Device), _Pool(Pool), _CmdBuf(CmdBuf), _Functions(Functions) {}

		void Reset();
		void Begin();
		void End();

		void BeginRenderPass(VkRenderPass renderPass, VkRect2D renderArea, VkFramebuffer framebuffer, uint32_t clearValuesCount, VkClearValue* clearValues);
		void EndRenderPass();

		void BindComputePipeline(const ComputePipeline* Pipeline);
		void BindGraphicsPipeline(const Graphics::GraphicsPipeline* Pipeline);
		void BindRayTracingPipeline(const RayTracingPipeline* Pipeline);

		void PushConstantsGraphics(const Graphics::GraphicsPipeline* pipeline, ShaderType stages, uint32_t offset, uint32_t size, const void* pValues);
		void BindDescriptorSetsGraphics(const Graphics::GraphicsPipeline* pipeline, uint32 firstSet, uint32 setCount, const VkDescriptorSet* sets);

		void PushConstantsRayTracing(const RayTracingPipeline* pipeline, ShaderType stages, uint32_t offset, uint32_t size, const void* pValues);
		void BindDescriptorSetsRayTracing(const RayTracingPipeline* pipeline, uint32 firstSet, uint32 setCount, const VkDescriptorSet* sets);

		void BindVertexBuffers(uint32_t first, uint32_t count, const BufferVulkan* buffers, const VkDeviceSize* offsets);
		void BindIndexBuffer(const BufferVulkan buffer, const VkDeviceSize offset);

		void Dispatch(uint32 X, uint32 Y, uint32 Z);
		void Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
		void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset, uint32 firstInstance);
		void TraceRays(const RayTracingPipeline* pipeline, uint32_t x, uint32_t y, uint32_t depth);

		const VkCommandBuffer& _GetHandle() const { return _CmdBuf; }

		~CommandBufferVulkan();
	};

}
