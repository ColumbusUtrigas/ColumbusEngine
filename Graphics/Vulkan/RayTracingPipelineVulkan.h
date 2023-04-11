#pragma once

#include <Graphics/RayTracingPipeline.h>
#include "PipelineDescriptorSetLayoutVulkan.h"
#include <vulkan/vulkan.h>

#include "BufferVulkan.h"

namespace Columbus
{

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
