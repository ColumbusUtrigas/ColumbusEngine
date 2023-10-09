#pragma once

#include <vector>
#include <string>

#include <Graphics/Core/Types.h> // from the engine

struct CompiledShaderBytecode
{
	Columbus::ShaderType Stage; // uint32_t
	uint32_t Flags;

	std::string EntryPoint;
	std::vector<uint8_t> Bytecode;
	// reflection data
};

struct CompiledShaderData
{
	uint32_t Flags;

	std::string Name;
	std::vector<CompiledShaderBytecode> Bytecodes;
	// reflection data, permutations
};

CompiledShaderData LoadCompiledShaderData(const std::string& Path);
void               SaveCompiledShaderData(const CompiledShaderData& Data, const std::string& Path);
