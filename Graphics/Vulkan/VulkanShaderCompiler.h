#pragma once

#include "Graphics/Core/Types.h"
#include <Graphics/Core/GraphicsCore.h>
#include <Core/fixed_vector.h>
#include <Core/SmartPointer.h>
#include <string>
#include <vector>
#include <span>
#include <vulkan/vulkan.h>

#include <ShaderBytecode/ShaderBytecode.h>

namespace Columbus
{

	// TODO: refactor/remove, shaderc compilation must stay, but moved to ShaderCompiler

	CompiledShaderBytecode CompileShaderStageFromSource_VK(const std::string& Source, const ShaderStageDesc& Stage, const std::string& Name, std::span<std::string> Defines);
	CompiledShaderData     CompileShaderPipelineFromSource_VK(const std::string& Source, const std::string& Name, ShaderLanguage Lang, std::span<ShaderStageDesc> Stages, std::span<std::string> Defines);

}