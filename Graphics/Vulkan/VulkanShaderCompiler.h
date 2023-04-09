#pragma once

#include <Graphics/Shader.h>
#include <Core/fixed_vector.h>
#include <Core/SmartPointer.h>
#include <string>
#include <vulkan/vulkan.h>

namespace Columbus
{

    // SPIR-V bytecode and reflection information
    struct CompiledSpirv
    {
        std::vector<uint32_t> Bytecode;

        fixed_vector<VkPushConstantRange, 16> pushConstants; // TODO
        fixed_vector<VkDescriptorSetLayoutCreateInfo, 16> descriptorSets; // TODO
    };

    CompiledSpirv CompileShaderStage_VK(SPtr<ShaderStage> stage, const std::string& name);

}