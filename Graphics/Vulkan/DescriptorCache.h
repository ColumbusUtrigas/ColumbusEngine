#pragma once

#include <vulkan/vulkan.h>

namespace Columbus
{

	class DeviceVulkan;

	class DescriptorCache
	{
	public:
		DescriptorCache(DeviceVulkan* Device) : Device(Device) {}

		void StartNewFrame()
		{

		}

		VkDescriptorSet GetDescriptorSet(VkPipeline Pipeline, int Index);
	private:
		DeviceVulkan* Device;
	};

}
