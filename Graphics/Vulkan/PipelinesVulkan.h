#pragma once

#include <Graphics/Core/Pipelines.h>

#include "PipelineDescriptorSetLayoutVulkan.h"
#include "BufferVulkan.h"
#include <vulkan/vulkan.h>

namespace Columbus
{

	class ComputePipelineVulkan : public ComputePipeline
	{
	public:
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout layout = VK_NULL_HANDLE;

		PipelineDescriptorSetLayoutsVulkan SetLayouts;

	public:
		ComputePipelineVulkan(const ComputePipelineDesc& Desc) : ComputePipeline(Desc) {}
	};

	class GraphicsPipelineVulkan : public GraphicsPipeline
	{
	public:
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout layout = VK_NULL_HANDLE;
		PipelineDescriptorSetLayoutsVulkan SetLayouts;

	public:
		GraphicsPipelineVulkan(const GraphicsPipelineDesc& desc) : GraphicsPipeline(desc) {}
	};

	class RayTracingPipelineVulkan : public RayTracingPipeline
	{
	public:
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout layout = VK_NULL_HANDLE;
		
		PipelineDescriptorSetLayoutsVulkan SetLayouts;

		Buffer* RayGenSBT = nullptr;
		Buffer* MissSBT = nullptr;
		Buffer* HitSBT = nullptr;
		Buffer* CallableSBT = nullptr;

		VkStridedDeviceAddressRegionKHR RayGenRegionSBT{};
		VkStridedDeviceAddressRegionKHR MissRegionSBT{};
		VkStridedDeviceAddressRegionKHR HitRegionSBT{};
		VkStridedDeviceAddressRegionKHR CallableRegionSBT{};

	public:
		RayTracingPipelineVulkan(const RayTracingPipelineDesc& Desc) : RayTracingPipeline(Desc) {}
	};

}
