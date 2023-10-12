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

	fixed_vector<VkDescriptorSetLayoutBinding, 16> Bindings;
	fixed_vector<VkDescriptorBindingFlags,     16> BindingFlags;
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

struct CompiledShaderData
{
	uint32_t Flags;
	std::string Name;

	std::vector<CompiledShaderBytecode> Bytecodes;
	// TODO: permutations

	SPtr<CompiledShaderBytecodeReflection> Reflection;
};

CompiledShaderData LoadCompiledShaderData(const std::string& Path);
void               SaveCompiledShaderData(const CompiledShaderData& Data, const std::string& Path);
void               ReflectCompiledShaderData(CompiledShaderData& Data);
