#pragma once

#include <string>
#include <vector>
#include <tuple>

namespace Columbus
{

	struct CompiledShader
	{
		std::string source;
	};

	struct CompiledProgram
	{
		std::vector<CompiledShader> shaders;
	};

	struct ShaderCompiler
	{
		// CSL2GLSL compute for now
		static CompiledProgram Compile(const std::string& source, const std::vector<std::string>& defines);
	};

}
