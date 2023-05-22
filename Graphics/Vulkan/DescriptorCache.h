#pragma once

#include <vulkan/vulkan.h>

namespace Columbus
{

	class DeviceVulkan;

	class DescriptorCache
	{
	public:
		DescriptorCache(DeviceVulkan* Device) : Device(Device) {}

		VkDescriptorSetLayout GetSetLayout();
	private:
		DeviceVulkan* Device;
	};

}
