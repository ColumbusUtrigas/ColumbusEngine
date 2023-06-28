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
		VkPipeline pipeline;
		VkPipelineLayout layout;

		PipelineDescriptorSetLayoutsVulkan SetLayouts;

	public:
		ComputePipelineVulkan(const ComputePipelineDesc& Desc) : ComputePipeline(Desc) {}
	};

	class GraphicsPipelineVulkan : public GraphicsPipeline
	{
	public:
		VkPipeline pipeline;
		VkPipelineLayout layout;
		PipelineDescriptorSetLayoutsVulkan SetLayouts;

	public:
		GraphicsPipelineVulkan(const GraphicsPipelineDesc& desc) : GraphicsPipeline(desc) {}
	};

	class RayTracingPipelineVulkan : public RayTracingPipeline
	{
	public:
		VkPipeline pipeline;
		VkPipelineLayout layout;
		
		PipelineDescriptorSetLayoutsVulkan SetLayouts;

		Buffer* RayGenSBT;
		Buffer* MissSBT;
		Buffer* HitSBT;
		Buffer* CallableSBT;

		VkStridedDeviceAddressRegionKHR RayGenRegionSBT;
		VkStridedDeviceAddressRegionKHR MissRegionSBT;
		VkStridedDeviceAddressRegionKHR HitRegionSBT;
		VkStridedDeviceAddressRegionKHR CallableRegionSBT;

	public:
		RayTracingPipelineVulkan(const RayTracingPipelineDesc& Desc) : RayTracingPipeline(Desc) {}
	};

}
