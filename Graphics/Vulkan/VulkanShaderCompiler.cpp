#include "VulkanShaderCompiler.h"

#include <Graphics/Core/GraphicsCore.h>
#include "Core/Assert.h"
#include "Graphics/Core/Types.h"
#include "ShaderBytecode/ShaderBytecode.h"
#include "TypeConversions.h"
#include "Common.h"

#include <System/File.h>

#include <shaderc/shaderc.hpp>
#include <Lib/SPIRV-Reflect/spirv_reflect.h>

#include <unordered_map>

namespace Columbus
{

	class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface
	{
	public:
		virtual shaderc_include_result* GetInclude(const char* requested_source,
		                                           shaderc_include_type type,
		                                           const char* requesting_source,
		                                           size_t include_depth) override
		{
			shaderc_include_result* result = nullptr;

			std::string source = requested_source;

			if (!source.ends_with(".glsl"))
			{
				source += ".glsl";
			}

			if (!LoadedIncludes.contains(source))
			{
				LoadedIncludes[source] = LoadShaderFile(source);
			}

			result = new shaderc_include_result();
			result->content = LoadedIncludes[source].c_str();
			result->content_length = LoadedIncludes[source].size();
			result->source_name = requested_source;
			result->source_name_length = strlen(requested_source);

			return result;
		}

		virtual void ReleaseInclude(shaderc_include_result* data) override
		{
			delete data;
		}

		virtual ~ShaderIncluder() override = default;

	private:
		std::unordered_map<std::string, std::string> LoadedIncludes;
	};

	CompiledShaderBytecode CompileShaderStageFromSource_VK(const std::string& Source, ShaderLanguage Lang, const ShaderStageDesc& Stage, const std::string& Name, std::span<std::string> Defines)
	{
		CompiledShaderBytecode result;
		result.EntryPoint = "main";
		result.Stage = Stage.Type;
		result.Flags = 0;

		shaderc::Compiler compiler;
		shaderc_shader_kind kind;
		shaderc::CompileOptions options;
		auto includer = std::make_unique<ShaderIncluder>();
		VkShaderStageFlagBits vkstage = ShaderTypeToVk(Stage.Type);

		for (const auto& Define : Defines)
		{
			options.AddMacroDefinition(Define);
		}

		options.SetIncluder(std::move(includer));

		switch (Lang)
		{
			case ShaderLanguage::GLSL: options.SetSourceLanguage(shaderc_source_language_glsl); break;
			case ShaderLanguage::HLSL: options.SetSourceLanguage(shaderc_source_language_hlsl); break;
			default: COLUMBUS_ASSERT_MESSAGE(false, "Unsupported shader language"); break;
		}

		switch (Stage.Type)
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
		auto spirv = compiler.CompileGlslToSpv(Source, kind, Name.c_str(), options);

		uint64_t spirvByteSize = (spirv.end() - spirv.begin()) * sizeof(u32);
		result.Bytecode.resize(spirvByteSize);
		memcpy(result.Bytecode.data(), spirv.begin(), spirvByteSize);

		if (spirv.GetNumErrors() == 0)
		{
			Log::Success("%s::%s, stage compiled", Name.c_str(), ShaderTypeToString(Stage.Type));
		}
		else
		{
			Log::Error("%s::%s, stage failed", spirv.GetErrorMessage().c_str(), ShaderTypeToString(Stage.Type));
		}

		return result;
	}

	CompiledShaderData CompileShaderPipelineFromSource_VK(const std::string& Source, const std::string& Name,
		ShaderLanguage Lang, std::span<ShaderStageDesc> Stages, std::span<std::string> Defines)
	{
		//COLUMBUS_ASSERT_MESSAGE(Lang == ShaderLanguage::GLSL, "Other languages than GLSL are not supported");

		CompiledShaderData Result;
		Result.Name = Name;
		Result.Flags = 0;

		for (const ShaderStageDesc& Stage : Stages)
		{
			Result.Bytecodes.push_back(CompileShaderStageFromSource_VK(Source, Lang, Stage, Name, Defines));
		}

		ReflectCompiledShaderData(Result);

		return Result;
	}

}
