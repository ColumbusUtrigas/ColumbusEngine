#include "VulkanShaderCompiler.h"

#include <Graphics/TypesCommon.h>
#include "TypeConversions.h"
#include "Common.h"

#include <shaderc/shaderc.hpp>
#include <Lib/SPIRV-Reflect/spirv_reflect.h>

namespace Columbus
{

	CompiledSpirv CompileShaderStage_VK(SPtr<ShaderStage> stage, const std::string& name)
	{
		CompiledSpirv result;
		shaderc::Compiler compiler;
		shaderc_shader_kind kind;
		shaderc::CompileOptions options;
		VkShaderStageFlagBits vkstage = ShaderTypeToVk(stage->Type);

		switch (stage->Type)
		{
			case ShaderType::Vertex:
				kind = shaderc_vertex_shader;
				options.AddMacroDefinition("VERTEX_SHADER");
				break;

			case ShaderType::Pixel:
				kind = shaderc_fragment_shader;
				options.AddMacroDefinition("PIXEL_SHADER");
				break;

			case ShaderType::Compute:
				kind = shaderc_compute_shader;
				options.AddMacroDefinition("COMPUTE_SHADER");
				break;

			case ShaderType::Raygen:
				kind = shaderc_raygen_shader;
				options.AddMacroDefinition("RAYGEN_SHADER");
				break;

			case ShaderType::Miss:
				kind = shaderc_miss_shader;
				options.AddMacroDefinition("MISS_SHADER");
				break;

			case ShaderType::ClosestHit:
				kind = shaderc_closesthit_shader;
				options.AddMacroDefinition("CLOSEST_HIT_SHADER");
				break;

			default: break;
		}

		options.SetTargetSpirv(shaderc_spirv_version_1_4);
		auto spirv = compiler.CompileGlslToSpv(stage->Source, kind, "name", options);
		uint64_t spirvByteSize = (spirv.end() - spirv.begin()) * 4;
		result.Bytecode = { spirv.begin(), spirv.end() };

		if (spirv.GetNumErrors() == 0)
		{
			Log::Success("%s::%s, stage compiled", name.c_str(), ShaderTypeToString(stage->Type));
		}
		else
		{
			Log::Error("%s::%s, stage failed", spirv.GetErrorMessage().c_str(), ShaderTypeToString(stage->Type));
		}

		// Generate reflection data for a shader
		SpvReflectShaderModule spv_module;
		SpvReflectResult reflect_result = spvReflectCreateShaderModule(spirvByteSize, spirv.begin(), &spv_module);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		// Enumerate and extract shader's input variables
		uint32_t input_count = 0;
		SpvReflectInterfaceVariable* inputs[128];

		reflect_result = spvReflectEnumerateInputVariables(&spv_module, &input_count, NULL);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		reflect_result = spvReflectEnumerateInputVariables(&spv_module, &input_count, inputs);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		// Enumerate and extract shader's push constants
		uint32_t push_constant_num = 0;
		SpvReflectBlockVariable* push_constants[128];

		reflect_result = spvReflectEnumeratePushConstantBlocks(&spv_module, &push_constant_num, NULL);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		reflect_result = spvReflectEnumeratePushConstantBlocks(&spv_module, &push_constant_num, push_constants);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		// Enumerate and extract shader's descriptor sets
		uint32_t sets_num = 0;
		SpvReflectDescriptorSet* sets[128];

		spvReflectEnumerateDescriptorSets(&spv_module, &sets_num, NULL);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		spvReflectEnumerateDescriptorSets(&spv_module, &sets_num, sets);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		// for (int i = 0; i < input_count; i++)
		// {
		// 	SpvReflectInterfaceVariable* input = inputs[i];
		// 	printf("%s, location(%i), storage(%i), decoration(%x), type(vec%i)\n", var->name, var->location, var->storage_class, var->decoration_flags, var->numeric.vector.component_count);
		// }

		for (int i = 0; i < push_constant_num; i++)
		{
			VkPushConstantRange push;
			push.stageFlags = vkstage;
			push.offset = push_constants[i]->offset;
			push.size = push_constants[i]->size;

			result.pushConstants.push_back(push);
		}			

		for (int i = 0; i < sets_num; i++)
		{
			DescriptorSetInfo setInfo;

			for (int b = 0; b < sets[i]->binding_count; b++)
			{
				auto binding = sets[i]->bindings[b];
				assert(stage->Type == ShaderType::Pixel || stage->Type == ShaderType::Raygen || stage->Type == ShaderType::ClosestHit);

				VkDescriptorType descriptorType;
				switch (binding->descriptor_type)
				{
					case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
						descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						break;

					case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
						descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
						break;

					case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
						descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
						break;

					case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
						descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
						break;

					default:
						assert(false && "Descriptor type is not supported");
						break;
				}

				VkDescriptorSetLayoutBinding bindingInfo;
				bindingInfo.binding = binding->binding;
				bindingInfo.descriptorType = descriptorType;
				bindingInfo.descriptorCount = binding->count;
				bindingInfo.stageFlags = vkstage;
				bindingInfo.pImmutableSamplers = nullptr;

				VkDescriptorBindingFlags bindingFlag = 0;

				// If it is an array of descriptors, we assume that it's size is variable and access is non-uniform
				if (binding->array.dims_count > 0)
				{
					COLUMBUS_ASSERT(b == (sets[i]->binding_count - 1)); // Variable descriptor must be the last in the set

					setInfo.VariableCountMax = binding->count;

					bindingFlag |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
				}

				setInfo.Bindings.push_back(bindingInfo);
				setInfo.BindingFlags.push_back(bindingFlag);
			}
			result.DescriptorSets.push_back(setInfo);
		}

		spvReflectDestroyShaderModule(&spv_module);

		return result;
	}

	ShaderStageBuildResultVulkan ShaderStageBuild_VK(SPtr<ShaderStage> stage, const std::string& name, VkDevice device)
	{
		ShaderStageBuildResultVulkan result;
		result.Spirv = CompileShaderStage_VK(stage, name);

		VkShaderModuleCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.codeSize = result.Spirv.Bytecode.size() * sizeof(uint32_t);
		info.pCode = result.Spirv.Bytecode.data();

		VkShaderModule module;

		VK_CHECK(vkCreateShaderModule(device, &info, nullptr, &module));

		result.ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		result.ShaderStageInfo.pNext = nullptr;
		result.ShaderStageInfo.flags = 0;
		result.ShaderStageInfo.stage = ShaderTypeToVk(stage->Type);
		result.ShaderStageInfo.module = module;
		result.ShaderStageInfo.pName = stage->EntryPoint.c_str();
		result.ShaderStageInfo.pSpecializationInfo = nullptr;
		return result;
	}

}
