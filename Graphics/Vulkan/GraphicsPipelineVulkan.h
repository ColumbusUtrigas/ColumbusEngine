#pragma once

#include <Graphics/GraphicsPipeline.h>
#include "PipelineDescriptorSetLayoutVulkan.h"
#include <vulkan/vulkan.h>

namespace Columbus
{

	class GraphicsPipelineVulkan : public GraphicsPipeline
	{
	public:
		VkPipeline pipeline;
		VkPipelineLayout layout;
		PipelineDescriptorSetLayoutsVulkan SetLayouts;

	public:
		GraphicsPipelineVulkan(const GraphicsPipelineDesc& desc) : GraphicsPipeline(desc) {}
	};

}
