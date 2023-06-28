#pragma once

#include <Graphics/Types.h>
#include <Core/fixed_vector.h>
#include <Core/SmartPointer.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace Columbus
{

	struct DescriptorSetInfo
	{
		uint32_t Index = 0;
		uint32_t VariableCountMax = 0;

		fixed_vector<VkDescriptorSetLayoutBinding, 16> Bindings;
		fixed_vector<VkDescriptorBindingFlags, 16> BindingFlags;
	};

	// SPIR-V bytecode and reflection information
	struct CompiledSpirv
	{
		std::vector<uint32_t> Bytecode;

		fixed_vector<VkPushConstantRange, 16> pushConstants; // TODO
		fixed_vector<DescriptorSetInfo, 16> DescriptorSets;
	};

	// Built shader module
	struct ShaderStageBuildResultVulkan
	{
		VkPipelineShaderStageCreateInfo ShaderStageInfo;
		CompiledSpirv Spirv;
	};

	CompiledSpirv CompileShaderStage_VK(SPtr<ShaderStage> stage, const std::string& name);

	ShaderStageBuildResultVulkan ShaderStageBuild_VK(SPtr<ShaderStage> stage, const std::string& name, VkDevice device);

}