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
	uint32_t Count;
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

	if (Size == 0)
	{
		return {};
	}

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

static std::vector<std::string> ReadStringVector(FILE* f)
{
	uint32_t Size = 0;
	fread(&Size, sizeof(Size), 1, f);

	std::vector<std::string> Result;
	Result.reserve(Size);

	for (uint32_t i = 0; i < Size; i++)
	{
		Result.push_back(ReadString(f));
	}

	return Result;
}

static std::vector<CompiledShaderPermutationAxis> ReadPermutationAxes(FILE* f)
{
	uint32_t Size = 0;
	fread(&Size, sizeof(Size), 1, f);

	std::vector<CompiledShaderPermutationAxis> Result;
	Result.reserve(Size);

	for (uint32_t i = 0; i < Size; i++)
	{
		CompiledShaderPermutationAxis Axis;
		Axis.Name = ReadString(f);
		fread(&Axis.MinValue, sizeof(Axis.MinValue), 1, f);
		fread(&Axis.MaxValue, sizeof(Axis.MaxValue), 1, f);
		Result.push_back(Axis);
	}

	return Result;
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

static void WriteStringVector(const std::vector<std::string>& Vector, FILE* f)
{
	uint32_t Size = (uint32_t)Vector.size();
	fwrite(&Size, sizeof(Size), 1, f);

	for (const std::string& String : Vector)
	{
		WriteString(String, f);
	}
}

static void WritePermutationAxes(const std::vector<CompiledShaderPermutationAxis>& Axes, FILE* f)
{
	uint32_t Size = (uint32_t)Axes.size();
	fwrite(&Size, sizeof(Size), 1, f);

	for (const CompiledShaderPermutationAxis& Axis : Axes)
	{
		WriteString(Axis.Name, f);
		fwrite(&Axis.MinValue, sizeof(Axis.MinValue), 1, f);
		fwrite(&Axis.MaxValue, sizeof(Axis.MaxValue), 1, f);
	}
}

static CompiledShaderBytecode ReadBytecode(FILE* f)
{
	CompiledShaderBytecode Bytecode;
	CompiledShaderBytecodeHeader BytecodeHeader;

	fread(&BytecodeHeader, sizeof(BytecodeHeader), 1, f);
	Bytecode.Stage = (Columbus::ShaderType)BytecodeHeader.Stage;
	Bytecode.Flags = BytecodeHeader.Flags;

	Bytecode.EntryPoint = ReadString(f);
	Bytecode.Bytecode = ReadStdVector<uint8_t>(f);

	return Bytecode;
}

static void WriteBytecode(const CompiledShaderBytecode& Bytecode, FILE* f)
{
	CompiledShaderBytecodeHeader BytecodeHeader{
		.Stage = (uint32_t)Bytecode.Stage,
		.Flags = Bytecode.Flags
	};

	fwrite(&BytecodeHeader, sizeof(BytecodeHeader), 1, f);
	WriteString(Bytecode.EntryPoint, f);
	WriteStdVector<uint8_t>(Bytecode.Bytecode, f);
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

	if (DataHeader.Version == 1)
	{
		CompiledShaderPermutation DefaultPermutation;
		DefaultPermutation.Name = "Default";
		for (uint32_t i = 0; i < DataHeader.Count; i++)
		{
			DefaultPermutation.Bytecodes.push_back(ReadBytecode(f));
		}

		Result.Permutations.push_back(DefaultPermutation);
	}
	else if (DataHeader.Version == 2)
	{
		for (uint32_t i = 0; i < DataHeader.Count; i++)
		{
			CompiledShaderPermutation Permutation;
			Permutation.Name = ReadString(f);
			Permutation.Defines = ReadStringVector(f);

			uint32_t NumBytecodes = 0;
			fread(&NumBytecodes, sizeof(NumBytecodes), 1, f);
			for (uint32_t BytecodeIndex = 0; BytecodeIndex < NumBytecodes; BytecodeIndex++)
			{
				Permutation.Bytecodes.push_back(ReadBytecode(f));
			}

			Result.Permutations.push_back(Permutation);
		}
	}
	else if (DataHeader.Version == 3)
	{
		Result.PermutationAxes = ReadPermutationAxes(f);

		for (uint32_t i = 0; i < DataHeader.Count; i++)
		{
			CompiledShaderPermutation Permutation;
			Permutation.Name = ReadString(f);
			Permutation.Defines = ReadStringVector(f);
			Permutation.AxisValues = ReadStdVector<int32_t>(f);

			uint32_t NumBytecodes = 0;
			fread(&NumBytecodes, sizeof(NumBytecodes), 1, f);
			for (uint32_t BytecodeIndex = 0; BytecodeIndex < NumBytecodes; BytecodeIndex++)
			{
				Permutation.Bytecodes.push_back(ReadBytecode(f));
			}

			Result.Permutations.push_back(Permutation);
		}
	}
	else
	{
		assert(false && "Unsupported compiled shader data version");
	}

	fclose(f);

	ReflectCompiledShaderData(Result);

	return Result;
}

void SaveCompiledShaderData(const CompiledShaderData& Data, const std::string& Path)
{
	CompiledShaderDataHeader DataHeader {
		.MagicNumber  = 0x44534343,
		.Version      = 3,
		.Flags        = Data.Flags,
		.Count        = (uint32_t)Data.Permutations.size(),
	};

	FILE* f = fopen(Path.c_str(), "wb");

	assert(!Data.Permutations.empty());

	fwrite(&DataHeader, sizeof(DataHeader), 1, f);
	WriteString(Data.Name, f);
	WritePermutationAxes(Data.PermutationAxes, f);

	for (const CompiledShaderPermutation& Permutation : Data.Permutations)
	{
		WriteString(Permutation.Name, f);
		WriteStringVector(Permutation.Defines, f);
		WriteStdVector<int32_t>(Permutation.AxisValues, f);

		uint32_t NumBytecodes = (uint32_t)Permutation.Bytecodes.size();
		fwrite(&NumBytecodes, sizeof(NumBytecodes), 1, f);
		for (const auto& Bytecode : Permutation.Bytecodes)
		{
			WriteBytecode(Bytecode, f);
		}
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

	for (uint32_t i = 0; i < push_constant_num; i++)
	{
		VkPushConstantRange push;
		push.stageFlags = ShaderTypeToVk(Bytecode.Stage); // TODO: information about push constant usage
		push.offset = push_constants[i]->offset;
		push.size = push_constants[i]->size;

		Reflection->PushConstants.push_back(push);
	}

	for (uint32_t i = 0; i < sets_num; i++)
	{
		DescriptorSetReflectionData setInfo;

		SpvReflectDescriptorType UsedDescriptorTypesCache[32]{(SpvReflectDescriptorType)-1};

		for (uint32_t b = 0; b < sets[i]->binding_count; b++)
		{
			auto binding = sets[i]->bindings[b];
			assert(Bytecode.Stage == ShaderType::Vertex || Bytecode.Stage == ShaderType::Pixel || Bytecode.Stage == ShaderType::Raygen || Bytecode.Stage == ShaderType::Miss || Bytecode.Stage == ShaderType::Anyhit || Bytecode.Stage == ShaderType::ClosestHit || Bytecode.Stage == ShaderType::Compute);

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

static SPtr<CompiledShaderBytecodeReflection> ReflectCompiledShaderBytecodes(std::vector<CompiledShaderBytecode>& Bytecodes)
{
	// TODO: link reflection data from multiple stages for a pipeline
	SPtr<CompiledShaderBytecodeReflection> Reflection = SPtr<CompiledShaderBytecodeReflection>(new CompiledShaderBytecodeReflection());

	for (CompiledShaderBytecode& Bytecode : Bytecodes)
	{
		ReflectCompiledShaderBytecode(Bytecode, Reflection);
	}

	return Reflection;
}

void ReflectCompiledShaderData(CompiledShaderData& Data)
{
	assert(!Data.Permutations.empty());

	for (CompiledShaderPermutation& Permutation : Data.Permutations)
	{
		Permutation.Reflection = ReflectCompiledShaderBytecodes(Permutation.Bytecodes);
	}
}
