#pragma once

#include <Graphics/GraphicsPipeline.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Columbus::Graphics
{

    class GraphicsPipelineVulkan : public GraphicsPipeline
    {
    public:
        VkPipeline pipeline;
        VkPipelineLayout layout;
        VkDescriptorSetLayout setLayouts[16]; // TODO

    public:
        GraphicsPipelineVulkan(const GraphicsPipelineDesc& desc) : GraphicsPipeline(desc) {}
    };

}
