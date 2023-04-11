#pragma once

#include <vulkan/vulkan.h>

namespace Columbus
{

	struct PipelineDescriptorSetLayoutsVulkan
	{
		// TODO: fixed_vector
		uint32_t UsedLayouts{};
		VkDescriptorSetLayout Layouts[16]{};
		uint32_t VariableCountMax[16]{};
	};

}
