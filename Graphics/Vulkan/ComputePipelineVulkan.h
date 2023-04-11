#include <Graphics/ComputePipeline.h>
#include "PipelineDescriptorSetLayoutVulkan.h"
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

}
