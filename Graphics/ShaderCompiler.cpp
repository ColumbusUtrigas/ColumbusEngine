#include <Graphics/ShaderCompiler.h>
#include <System/Log.h>
#include <Core/fixed_vector.h>
#include <sstream>
#include <regex>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <iostream>

namespace Columbus::Graphics
{

const std::string CommonShaderHeader_GL =
R"(#define Texture2D sampler2D
#define Texture3D sampler3D
#define TextureCube samplerCube

#define Texture2DMS sampler2DMS
#define Texture2DShadow sampler2DShadow

#define saturate(x) clamp(x, 0, 1)

#define float2 vec2
#define float3 vec3
#define float4 vec4

#define int2 ivec2
#define int3 ivec3
#define int4 ivec4

#define float2x2 mat2x2
#define float2x3 mat2x3
#define float2x4 mat2x4
#define float3x2 mat3x2
#define float3x3 mat3x3
#define float3x4 mat3x4
#define float4x2 mat4x2
#define float4x3 mat4x3
#define float4x4 mat4x4

#define begin_cbv(name) layout(std140) uniform name {
#define end_cbv };

#define begin_uav(name,  slot) layout(std430, binding = slot) buffer name {
#define end_uav(name) };

#if __VERSION__ < 130
	#define Sample2D(tex, uv) texture2D(tex, uv)
	#define Sample3D(tex, uv) texture3D(tex, uv)
	#define SampleCube(tex, uv) textureCube(tex, uv)

	#define Sample2DLod(tex, uv, lod) texture2DLod(tex, uv, lod)
	#define Sample3DLod(tex, uv, lod) texture3DLod(tex, uv, lod)
	#define SampleCubeLod(tex, uv, lod) textureCubeLod(tex, uv, lod)
	#define Sample2DShadow(tex, uvc) shadow2D(tex, uvc)
#else
	#define Sample2D(tex, uv) texture(tex, uv)
	#define Sample3D(tex, uv) texture(tex, uv)
	#define SampleCube(tex, uv) texture(tex, uv)

	#define Sample2DLod(tex, uv, lod) textureLod(tex, uv, lod)
	#define Sample3DLod(tex, uv, lod) textureLod(tex, uv, lod)
	#define SampleCubeLod(tex, uv, lod) textureLod(tex, uv, lod)

	#define Sample2DMS(tex, uv, lod) texelFetch(tex, uv, lod)
	#define Sample2DShadow(tex, uvc) texture(tex, uvc)
#endif
)";

const std::string CommonShaderHeader_DX12 =
R"(
#define begin_cbv(name) cbuffer name {
#define end_cbv }
)";

const std::string ComputeShaderHeader_GL =
R"(
)";

	enum class SemanticType
	{
		Float,
		Float2,
		Float3,
		Float4
	};

	enum class ShaderResourceType
	{
		Texture2D
	};

	struct Semantic
	{
		SemanticType type;
		std::string name;
		std::string semantic;
		int slot;
	};

	struct ShaderResource
	{
		ShaderResourceType type;
		std::string name;
		int slot;
	};

	using ShaderInput  = fixed_vector<Semantic, 16>;
	using ShaderOutput = fixed_vector<Semantic, 16>;
	using ShaderResources = fixed_vector<ShaderResource, 32>;

	struct CompilerShaderStageData
	{
		ShaderType type;
		ShaderInput input;
		ShaderOutput output;
		ShaderResources resources;
		std::string source;
		std::string entry;
	};

	struct CompilerShaderData
	{
		static constexpr int idVertex = 0;
		static constexpr int idPixel = 1;
		static constexpr int idCompute = 5;

		SPtr<CompilerShaderStageData> stages[6];
		SPtr<CompilerShaderStageData> vs = stages[idVertex];
		SPtr<CompilerShaderStageData> ps = stages[idPixel];
		SPtr<CompilerShaderStageData> cs = stages[idCompute];
	};

	SemanticType SemanticTypeFromHlsl(const std::string& str)
	{
		if (str == "float")  return SemanticType::Float;
		if (str == "float2") return SemanticType::Float2;
		if (str == "float3") return SemanticType::Float3;
		if (str == "float4") return SemanticType::Float4;
	}

	std::string SemanticTypeToHlsl(SemanticType type)
	{
		switch (type)
		{
		case SemanticType::Float:  return "float";
		case SemanticType::Float2: return "float2";
		case SemanticType::Float3: return "float3";
		case SemanticType::Float4: return "float4";
		}
	}

	std::string SemanticTypeToGlsl(SemanticType type)
	{
		switch (type)
		{
		case SemanticType::Float:  return "float";
		case SemanticType::Float2: return "vec2";
		case SemanticType::Float3: return "vec3";
		case SemanticType::Float4: return "vec4";
		}
	}

	template <typename  T>
	std::string WriteSignatureHeader(const T& sig)
	{
		std::string result;

		struct
		{
			std::string semantic(std::string str) { str.resize(20, ' '); return str; }
			std::string name(std::string str) { str.resize(15, ' '); return str; }
			std::string type(std::string str) { str.resize(10, ' ');  return str; }
			std::string slot(std::string str) { str.resize(4, ' ');  return str; }
			std::string slot(int val) { return slot(std::to_string(val)); }
		} sizes;

		result += "// " + sizes.semantic("SEMANTIC") + sizes.name("NAME") + sizes.type("TYPE") + sizes.slot("SLOT") + "\n";
		result += "//\n";
		for (const auto& sem : sig)
		{
			auto typestr = SemanticTypeToHlsl(sem.type);
			result += "// " + sizes.semantic(sem.semantic) + sizes.name(sem.name) + sizes.type(typestr) + sizes.slot(sem.slot) + '\n';
		}
		result += "//\n";

		return result;
	}

	std::string InputName_GLSL(const std::string& input)
	{
		return "i_" + input;
	}

	std::string OutputName_GLSL(const std::string& output)
	{
		return "o_" + output;
	}

	template <typename T>
	std::string WriteSignature_HLSL(const T& sig, const std::string& name)
	{
		std::string result = WriteSignatureHeader(sig);

		result += "struct " + name + "\n{\n";
		for (const auto& sem : sig)
		{
			result += "\t" + SemanticTypeToHlsl(sem.type) + ' ' + sem.name + " : " + sem.semantic + ";\n";
		}
		result += "};\n";

		return result;
	}

	template <typename T>
	std::string WriteSignature_GLSL(const T& sig, const std::string& name, bool input)
	{
		std::string result = WriteSignatureHeader(sig);

		for (const auto& sem : sig)
		{
			auto typestr = SemanticTypeToGlsl(sem.type);
			auto qualifier = input ? ") in " : ") out ";
			auto name = input ? InputName_GLSL(sem.name) : OutputName_GLSL(sem.name);
			result += "layout (location = " + std::to_string(sem.slot) + qualifier + typestr + ' ' + name + ";\n";
		}

		result += "\nstruct " + name + "\n{\n";
		for (const auto& sem : sig)
		{
			result += "\t" + SemanticTypeToGlsl(sem.type) + ' ' + sem.name + ";\n";
		}
		result += "};\n";

		return result;
	}

	const std::regex re_shader(R"(#shader (vertex|pixel|compute) (.+))");
	const std::regex re_input(R"(#input\((.+), (.+), (.+), (.+)\))");
	const std::regex re_output(R"(#output\((.+), (.+), (.+), (.+)\))");
	const std::regex re_texture(R"(#texture\((.+), (.+), (.+)\))");

	void Preprocess(SPtr<CompilerShaderData> data, const std::string& src)
	{
		std::stringstream s(src);

		SPtr<CompilerShaderStageData> current;

		for (std::string line; std::getline(s, line); )
		{
			std::smatch match;

			if (std::regex_match(line, match, re_shader))
			{
				if (match.str(1) == "vertex")
				{
					data->vs = std::make_shared<CompilerShaderStageData>();
					current = data->vs;
					current->type = ShaderType::Vertex;
					current->entry = match.str(2);
				}
				else if (match.str(1) == "pixel")
				{
					data->ps = std::make_shared<CompilerShaderStageData>();
					current = data->ps;
					current->type = ShaderType::Pixel;
					current->entry = match.str(2);
				}
				else if (match.str(1) == "compute")
				{
					data->cs = std::make_shared<CompilerShaderStageData>();
					current = data->cs;
					current->type = ShaderType::Compute;
					current->entry = match.str(2);
				}
				else
				{
					Log::Error("Undefined #shader stage");
				}
			}
			else if (std::regex_match(line, match, re_input))
			{
				auto type = SemanticTypeFromHlsl(match.str(1));
				current->input.push_back(Semantic{ type, match.str(2), match.str(3), std::stoi(match.str(4)) });
			}
			else if (std::regex_match(line, match, re_output))
			{
				auto type = SemanticTypeFromHlsl(match.str(1));
				current->output.push_back(Semantic{ type, match.str(2), match.str(3), std::stoi(match.str(4)) });
			}
			else if (std::regex_match(line, match, re_texture))
			{
				
			}
			else
			{
				if (!current)
				{
				}
				else
				{
					current->source += line + '\n';
				}
			}
		}

		#undef current
	}

	std::map<ShaderType, std::string> shader_input_names
	{
		{ ShaderType::Vertex, "VS_INPUT" },
		{ ShaderType::Pixel, "PS_INPUT" }
	};

	std::map<ShaderType, std::string> shader_output_names
	{
		{ ShaderType::Vertex, "VS_OUTPUT" },
		{ ShaderType::Pixel, "PS_OUTPUT" }
	};

	SPtr<ShaderStage> Generate_HLSL(SPtr<CompilerShaderStageData> stage)
	{
		if (!stage) return nullptr;

		auto result = std::make_shared<ShaderStage>();
		result->Language = ShaderLanguage::HLSL;
		result->EntryPoint = stage->entry;
		result->Type = stage->type;
		result->Source += CommonShaderHeader_DX12;

		if (stage->type != ShaderType::Compute)
		{
			result->Source += "// Input signature:\n//\n";
			result->Source += WriteSignature_HLSL(stage->input, shader_input_names[stage->type]);
			result->Source += "\n// Output signature:\n//\n";
			result->Source += WriteSignature_HLSL(stage->output, shader_output_names[stage->type]);
		}
		result->Source += stage->source;

		return result;
	}

	SPtr<ShaderStage> Generate_GLSL(SPtr<CompilerShaderStageData> stage)
	{
		if (!stage) return nullptr;

		auto& input = shader_input_names[stage->type];
		auto& output = shader_output_names[stage->type];

		auto result = std::make_shared<ShaderStage>();
		result->Language = ShaderLanguage::GLSL;
		result->EntryPoint = "main";
		result->Type = stage->type;

		result->Source += "#version 430 core\n";
		result->Source += CommonShaderHeader_GL;

		if (stage->type != ShaderType::Compute)
		{
			result->Source += "\n// Input signature:\n//\n";
			result->Source += WriteSignature_GLSL(stage->input, input, true);
			result->Source += "\n// Output signature:\n//\n";
			result->Source += WriteSignature_GLSL(stage->output, output, false);
		}
		else
		{
			result->Source += ComputeShaderHeader_GL;
		}

		result->Source += stage->source;

		if (stage->type != ShaderType::Compute)
		{
			result->Source += "\nvoid main()\n{\n";
			result->Source += "\t" + input + " i;\n";
			result->Source += "\t// write input\n";
			for (const auto& sem : stage->input)
			{
				result->Source += "\ti." + sem.name + " = " + InputName_GLSL(sem.name) + ";\n";
			}
			result->Source += "\t" + output + " o = " + stage->entry + "(i);\n";
			result->Source += "\t// write output\n";
			for (const auto& sem : stage->output)
			{
				if (stage->type == ShaderType::Vertex && sem.semantic == "SV_POSITION")
				{
					result->Source += "\tgl_Position = o." + sem.name + ";\n";
				}

				result->Source += "\t" + OutputName_GLSL(sem.name) + " = o." + sem.name + ";\n";
			}
			result->Source += "}\n";
		}

		return result;
	}

	CompiledProgram ShaderCompiler::Compile(const std::string& source, ShaderLanguage targetLang, const std::vector<std::string>& defines)
	{
		CompiledProgram prog;

		auto data = std::make_shared<CompilerShaderData>();
		Preprocess(data, source);

		switch (targetLang)
		{
		case ShaderLanguage::HLSL:
			prog.VS = Generate_HLSL(data->vs);
			prog.PS = Generate_HLSL(data->ps);
			prog.CS = Generate_HLSL(data->cs);
			break;
		case ShaderLanguage::GLSL:
			prog.VS = Generate_GLSL(data->vs);
			prog.PS = Generate_GLSL(data->ps);
			prog.CS = Generate_GLSL(data->cs);
			break;
		}

		return prog;
	}

}
