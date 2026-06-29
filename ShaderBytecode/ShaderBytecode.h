#pragma once

#include <vector>
#include <string>

// from the engine
#include <Core/Core.h>
#include <Graphics/Core/Types.h>
#include <Core/fixed_vector.h>

#include <vulkan/vulkan.h>

struct DescriptorSetReflectionData
{
	uint32_t Index = 0;
	uint32_t VariableCountMax = 0;

	fixed_vector<VkDescriptorSetLayoutBinding, 32> Bindings;
	fixed_vector<VkDescriptorBindingFlags,     32> BindingFlags;
};

struct CompiledShaderBytecodeReflection
{
	fixed_vector<VkPushConstantRange,         16> PushConstants; // TODO
	fixed_vector<DescriptorSetReflectionData, 16> DescriptorSets;
};

struct CompiledShaderBytecode
{
	Columbus::ShaderType Stage; // uint32_t
	uint32_t Flags;

	std::string EntryPoint;
	std::vector<uint8_t> Bytecode;
};

struct CompiledShaderPermutationAxis
{
	std::string Name;
	int32_t MinValue = 0;
	int32_t MaxValue = 0;
};

struct CompiledShaderPermutation
{
	std::string Name;
	std::vector<std::string> Defines;
	std::vector<int32_t> AxisValues;
	std::vector<CompiledShaderBytecode> Bytecodes;

	SPtr<CompiledShaderBytecodeReflection> Reflection;
};

struct CompiledShaderData
{
	uint32_t Flags;
	std::string Name;

	std::vector<CompiledShaderPermutationAxis> PermutationAxes;
	std::vector<CompiledShaderPermutation> Permutations;
};

CompiledShaderData LoadCompiledShaderData(const std::string& Path);
void               SaveCompiledShaderData(const CompiledShaderData& Data, const std::string& Path);
void               ReflectCompiledShaderData(CompiledShaderData& Data);
