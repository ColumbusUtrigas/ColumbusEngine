#include <Graphics/ComputePipeline.h>
#include <vulkan/vulkan.h>

namespace Columbus
{

    class ComputePipelineVulkan : public ComputePipeline
    {
    public:
        VkPipeline pipeline;
		VkPipelineLayout layout;
		VkDescriptorSetLayout setLayouts[16]; // TODO

    public:
        ComputePipelineVulkan(const ComputePipelineDesc& Desc) : ComputePipeline(Desc) {}
    };

}
