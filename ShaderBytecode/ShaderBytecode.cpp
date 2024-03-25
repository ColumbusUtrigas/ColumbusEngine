#include "ShaderBytecode.h"
#include <cstdint>
#include <cstdio>

#include <Graphics/Vulkan/TypeConversions.h>

#include <Lib/SPIRV-Reflect/spirv_reflect.h>
#include <vulkan/vulkan_core.h>

using namespace Columbus;

struct CompiledShaderDataHeader
{
	uint32_t MagicNumber; // CCSD = 0x44534343 LE
	uint32_t Version;
	uint32_t Flags;
	uint32_t NumBytecodes;
};

struct CompiledShaderBytecodeHeader
{
	uint32_t Stage; // ShaderType 
	uint32_t Flags;
};

template <typename T>
static std::vector<T> ReadStdVector(FILE* f)
{
	uint32_t Size = 0;

	fread(&Size, sizeof(Size), 1, f);

	T* Data = (T*)malloc(Size * sizeof(T));
	fread(Data, sizeof(T), Size, f);

	std::vector<T> Result(Data, Data + Size);
	free(Data);
	return Result;
}

static std::string ReadString(FILE* f)
{
	static constexpr int StringBufSize = 2048;

	uint32_t Size = 0;
	char StringBuf[StringBufSize]{0};

	fread(&Size, sizeof(Size), 1, f);
	assert(Size < StringBufSize);
	fread(StringBuf, sizeof(char), Size, f);

	return std::string(StringBuf);
}

template <typename T>
static void WriteStdVector(const std::vector<T>& Vector, FILE* f)
{
	uint32_t Size = (uint32_t)Vector.size();

	fwrite(&Size, sizeof(Size), 1, f); // write size
	fwrite(Vector.data(), sizeof(T), Vector.size(), f); // write data
}

static void WriteString(const std::string& String, FILE* f)
{
	uint32_t Size = (uint32_t)String.size();

	fwrite(&Size, sizeof(Size), 1, f); // write size
	fwrite(String.c_str(), sizeof(char), String.size(), f); // write string, not null-terminated
}

CompiledShaderData LoadCompiledShaderData(const std::string& Path)
{
	CompiledShaderData Result;

	CompiledShaderDataHeader DataHeader;

	FILE* f = fopen(Path.c_str(), "rb");

	fread(&DataHeader, sizeof(DataHeader), 1, f);
	Result.Name = ReadString(f);
	Result.Flags = DataHeader.Flags;

	// TODO: error handling

	for (int i = 0; i < DataHeader.NumBytecodes; i++)
	{
		CompiledShaderBytecode Bytecode;
		CompiledShaderBytecodeHeader BytecodeHeader;

		fread(&BytecodeHeader, sizeof(BytecodeHeader), 1, f);
		Bytecode.Stage = (Columbus::ShaderType)BytecodeHeader.Stage;
		Bytecode.Flags = BytecodeHeader.Flags;

		Bytecode.EntryPoint = ReadString(f);
		Bytecode.Bytecode = ReadStdVector<uint8_t>(f);

		Result.Bytecodes.push_back(Bytecode);
	}

	fclose(f);

	ReflectCompiledShaderData(Result);

	return Result;
}

void SaveCompiledShaderData(const CompiledShaderData& Data, const std::string& Path)
{
	CompiledShaderDataHeader DataHeader {
		.MagicNumber  = 0x44534343,
		.Version      = 1,
		.Flags        = Data.Flags,
		.NumBytecodes = (uint32_t)Data.Bytecodes.size(),
	};

	FILE* f = fopen(Path.c_str(), "wb");

	fwrite(&DataHeader, sizeof(DataHeader), 1, f);
	WriteString(Data.Name, f);

	for (const auto& Bytecode : Data.Bytecodes)
	{
		CompiledShaderBytecodeHeader BytecodeHeader{
			.Stage = (uint32_t)Bytecode.Stage,
			.Flags = Bytecode.Flags
		};

		fwrite(&BytecodeHeader, sizeof(BytecodeHeader), 1, f);
		WriteString(Bytecode.EntryPoint, f);
		WriteStdVector<uint8_t>(Bytecode.Bytecode, f);
	}

	fclose(f);
}

static void ReflectCompiledShaderBytecode(CompiledShaderBytecode& Bytecode, SPtr<CompiledShaderBytecodeReflection> Reflection)
{
	// Generate reflection data for a shader
	SpvReflectShaderModule spv_module;
	SpvReflectResult reflect_result = spvReflectCreateShaderModule(Bytecode.Bytecode.size(), Bytecode.Bytecode.data(), &spv_module);
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
	assert(sets_num < 128);

	spvReflectEnumerateDescriptorSets(&spv_module, &sets_num, sets);
	assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

	for (int i = 0; i < push_constant_num; i++)
	{
		VkPushConstantRange push;
		push.stageFlags = ShaderTypeToVk(Bytecode.Stage); // TODO: information about push constant usage
		push.offset = push_constants[i]->offset;
		push.size = push_constants[i]->size;

		Reflection->PushConstants.push_back(push);
	}

	for (int i = 0; i < sets_num; i++)
	{
		DescriptorSetReflectionData setInfo;

		SpvReflectDescriptorType UsedDescriptorTypesCache[32]{(SpvReflectDescriptorType)-1};

		for (int b = 0; b < sets[i]->binding_count; b++)
		{
			auto binding = sets[i]->bindings[b];
			assert(Bytecode.Stage == ShaderType::Vertex || Bytecode.Stage == ShaderType::Pixel || Bytecode.Stage == ShaderType::Raygen || Bytecode.Stage == ShaderType::ClosestHit || Bytecode.Stage == ShaderType::Compute);

			bool skipBinding = false;

			// we don't need a designated sampler if we already have a combined image sampler for this binding
			if (binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER && UsedDescriptorTypesCache[binding->binding] == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				skipBinding = true;
			}

			if (!skipBinding)
			{
				UsedDescriptorTypesCache[binding->binding] = binding->descriptor_type;

				VkDescriptorType descriptorType;
				switch (binding->descriptor_type)
				{
					case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
						descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
						break;

					case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
						descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
						break;

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

					case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
						descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						break;

					default:
						assert(false && "Descriptor type is not supported");
						break;
				}

				VkDescriptorSetLayoutBinding bindingInfo;
				bindingInfo.binding = binding->binding;
				bindingInfo.descriptorType = descriptorType;
				bindingInfo.descriptorCount = binding->count;
				// bindingInfo.stageFlags = vkstage;
				bindingInfo.stageFlags = VK_SHADER_STAGE_ALL;
				bindingInfo.pImmutableSamplers = nullptr;

				VkDescriptorBindingFlags bindingFlag = 0;

				// If it is an array of descriptors, we assume that it's size is variable and access is non-uniform
				if (binding->array.dims_count > 0)
				{
					assert(b == (sets[i]->binding_count - 1)); // Variable descriptor must be the last in the set

					setInfo.VariableCountMax = binding->count;

					bindingFlag |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
				}

				setInfo.Bindings.push_back(bindingInfo);
				setInfo.BindingFlags.push_back(bindingFlag);
			}
		}

		Reflection->DescriptorSets.push_back(setInfo);
	}

	spvReflectDestroyShaderModule(&spv_module);
}

void ReflectCompiledShaderData(CompiledShaderData& Data)
{
	// TODO: link reflection data from multiple stages for a pipeline
	Data.Reflection = SPtr<CompiledShaderBytecodeReflection>(new CompiledShaderBytecodeReflection());

	for (CompiledShaderBytecode& Bytecode : Data.Bytecodes)
	{
		ReflectCompiledShaderBytecode(Bytecode, Data.Reflection);
	}
}