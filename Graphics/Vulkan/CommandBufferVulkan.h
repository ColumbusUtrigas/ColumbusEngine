#pragma once

#include <Graphics/Core/GraphicsCore.h>
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
		CommandBufferVulkan(VkDevice Device, VkCommandPool Pool, VkCommandBuffer CmdBuf, VulkanFunctions& Functions);

		void Reset();
		void Begin();
		void End();

		void BeginDebugMarker(const char* Name);
		void EndDebugMarker();
		void InsertDebugLabel(const char* Name);

		void BeginRenderPass(VkRenderPass renderPass, VkRect2D renderArea, VkFramebuffer framebuffer, uint32_t clearValuesCount, VkClearValue* clearValues);
		void EndRenderPass();

		void BindComputePipeline(const ComputePipeline* Pipeline);
		void BindGraphicsPipeline(const GraphicsPipeline* Pipeline);
		void BindRayTracingPipeline(const RayTracingPipeline* Pipeline);

		void PushConstantsCompute(const ComputePipeline* pipeline, ShaderType stages, uint32_t offset, uint32_t size, const void* pValues);
		void BindDescriptorSetsCompute(const ComputePipeline* Pipeline, uint32 First, uint32 Count, const VkDescriptorSet* Sets);

		void PushConstantsGraphics(const GraphicsPipeline* pipeline, ShaderType stages, uint32_t offset, uint32_t size, const void* pValues);
		void BindDescriptorSetsGraphics(const GraphicsPipeline* pipeline, uint32 firstSet, uint32 setCount, const VkDescriptorSet* sets);

		void PushConstantsRayTracing(const RayTracingPipeline* pipeline, ShaderType stages, uint32_t offset, uint32_t size, const void* pValues);
		void BindDescriptorSetsRayTracing(const RayTracingPipeline* pipeline, uint32 firstSet, uint32 setCount, const VkDescriptorSet* sets);

		void BindVertexBuffers(uint32_t first, uint32_t count, const Buffer** buffers, const VkDeviceSize* offsets);
		void BindIndexBuffer(const Buffer* buffer, const VkDeviceSize offset);

		void SetViewport(float X, float Y, float Width, float Height, float MinDepth, float MaxDepth);
		void SetScissor(i32 X, i32 Y, u32 Width, u32 Height);

		void Dispatch(uint32 X, uint32 Y, uint32 Z);
		void Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
		void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset, uint32 firstInstance);
		void TraceRays(const RayTracingPipeline* pipeline, uint32_t x, uint32_t y, uint32_t depth);

		// TODO: layout api
		void TransitionImageLayout(Texture2* Texture, VkImageLayout NewLayout);
		void CopyImage(const Texture2* Src, const Texture2* Dst, iVector3 SrcOffset, iVector3 DstOffset, iVector3 Size);
		void CopyBuffer(const Buffer* Src, const Buffer* Dst, u64 SrcOffset, u64 DstOffset, u64 Size);

		void ResetQueryPool(const QueryPool* Pool, u32 FirstQuery, u32 QueryCount);
		void WriteTimestamp(const QueryPool* Pool, u32 Id);

		const VkCommandBuffer& _GetHandle() const { return _CmdBuf; }

		~CommandBufferVulkan();
	};

}
