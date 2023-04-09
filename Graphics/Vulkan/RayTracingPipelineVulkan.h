#pragma once

#include <Graphics/RayTracingPipeline.h>
#include <vulkan/vulkan.h>

#include "BufferVulkan.h"

namespace Columbus
{

	class RayTracingPipelineVulkan : public RayTracingPipeline
	{
	public:
		VkPipeline pipeline;
		VkPipelineLayout layout;
		VkDescriptorSetLayout setLayouts[16]; // TODO

		BufferVulkan RayGenSBT;
		BufferVulkan MissSBT;
		BufferVulkan HitSBT;
		BufferVulkan CallableSBT;

		VkStridedDeviceAddressRegionKHR RayGenRegionSBT;
		VkStridedDeviceAddressRegionKHR MissRegionSBT;
		VkStridedDeviceAddressRegionKHR HitRegionSBT;
		VkStridedDeviceAddressRegionKHR CallableRegionSBT;

	public:
		RayTracingPipelineVulkan(const RayTracingPipelineDesc& Desc) : RayTracingPipeline(Desc) {}
	};

}
