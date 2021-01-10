#pragma once

#include <Graphics/Types.h>
#include <Core/SmartPointer.h>
#include <string>
#include <vector>
#include <tuple>

namespace Columbus::Graphics
{

	struct CompiledShader
	{
		std::string source;
	};

	struct CompiledProgram
	{
		SPtr<ShaderStage> VS;
		SPtr<ShaderStage> PS;
		SPtr<ShaderStage> CS;

		std::vector<CompiledShader> shaders;
	};

	struct ShaderCompiler
	{
		// CSL to targetLang
		static CompiledProgram Compile(const std::string& source, ShaderLanguage targetLang, const std::vector<std::string>& defines);
	};

}
