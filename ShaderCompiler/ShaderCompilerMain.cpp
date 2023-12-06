#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <format>
#include <fstream>

#include <ShaderBytecode/ShaderBytecode.h>

using namespace Columbus;

// supported shader types: vs, ps, gs, hs, ds, cs, rgen, rint, rahit, rchit, rmiss

ShaderType StringToShaderType(const std::string& Type)
{
	if (Type == "vs") return ShaderType::Vertex;
	if (Type == "ps") return ShaderType::Pixel;
	if (Type == "gs") return ShaderType::Geometry;
	if (Type == "hs") return ShaderType::Hull;
	if (Type == "ds") return ShaderType::Domain;

	if (Type == "cs") return ShaderType::Compute;

	if (Type == "rgen")  return ShaderType::Raygen;
	if (Type == "rint")  return ShaderType::Intersection;
	if (Type == "rahit") return ShaderType::Anyhit;
	if (Type == "rchit") return ShaderType::ClosestHit;
	if (Type == "rmiss") return ShaderType::Miss;

	printf("Fatal error: shader type %s is not recognised\n", Type.c_str());
	exit(1);
}

std::string DefineForShaderStage(ShaderType Stage)
{
	switch (Stage)
	{
		case ShaderType::Vertex:       return "VERTEX_SHADER";
		case ShaderType::Pixel:	       return "PIXEL_SHADER";
		case ShaderType::Hull:         return "HULL_SHADER";
		case ShaderType::Domain:       return "DOMAIN_SHADER";
		case ShaderType::Geometry:     return "GEOMETRY_SHADER";
		case ShaderType::Compute:      return "COMPUTE_SHADER";
		case ShaderType::Raygen:       return "RAYGEN_SHADER";
		case ShaderType::Miss:         return "MISS_SHADER";
		case ShaderType::Anyhit:       return "ANYHIT_SHADER";
		case ShaderType::ClosestHit:   return "CLOSEST_HIT_SHADER";
		case ShaderType::Intersection: return "INTERSECTION_SHADER";
		default:
			printf("Fatal error: unrecognised shader stage %i\n", int(Stage));
			exit(1);
	}
}

std::string ShaderStageToGlslangStage(ShaderType Stage)
{
	switch (Stage)
	{
	case ShaderType::Vertex:       return "vert";
	case ShaderType::Pixel:	       return "frag";
	case ShaderType::Hull:         return "tese";
	case ShaderType::Domain:       return "tesc";
	case ShaderType::Geometry:     return "geom";
	case ShaderType::Compute:      return "comp";
	case ShaderType::Raygen:       return "rgen";
	case ShaderType::Miss:         return "rmiss";
	case ShaderType::Anyhit:       return "rahit";
	case ShaderType::ClosestHit:   return "rchit";
	case ShaderType::Intersection: return "rint";
	default:
		printf("Fatal error: unrecognised shader stage %i\n", int(Stage));
		exit(1);
	}
}

std::string ShaderStageToDxcStage(ShaderType Stage)
{
	std::string profile_version = "6_3";

	switch (Stage)
	{
	case ShaderType::Vertex:       return "vs_" + profile_version;
	case ShaderType::Pixel:	       return "ps_" + profile_version;
	case ShaderType::Hull:         return "hs_" + profile_version;
	case ShaderType::Domain:       return "ds_" + profile_version;
	case ShaderType::Geometry:     return "gs_" + profile_version;
	case ShaderType::Compute:      return "cs_" + profile_version;
	case ShaderType::Raygen:       return "lib_" + profile_version;
	case ShaderType::Miss:         return "lib_" + profile_version;
	case ShaderType::Anyhit:       return "lib_" + profile_version;
	case ShaderType::ClosestHit:   return "lib_" + profile_version;
	case ShaderType::Intersection: return "lib_" + profile_version;
	default:
		printf("Fatal error: unrecognised shader stage %i\n", int(Stage));
		exit(1);
	}
}

std::vector<std::string> DivideStringBy(std::string s, std::string delimiter)
{
	std::vector<std::string> result;

	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos)
	{
		token = s.substr(0, pos);
		result.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	result.push_back(s);

	return result;
}

CompiledShaderBytecode CompileStage(const std::string& Path, const std::string& TmpOutput, const std::string& EntryPoint, const std::vector<std::string>& Defines, ShaderType Stage)
{
	std::string DefinesParameters;
	for (const auto& Define : Defines)
	{
		DefinesParameters += " -D" + Define;
	}

	std::string CommandLine;

	if (Path.ends_with(".hlsl"))
	{
		printf("Compiling as HLSL\n");

		const char* DxcPath = getenv("DXC_PATH");
		printf("DXC_PATH is %s\n", DxcPath ? DxcPath : "not specified");

		std::string SpvExtensions;
		// -fspv-reflect requires these
		// SpvExtensions += "-fspv-extension=SPV_GOOGLE_hlsl_functionality1 ";
		// SpvExtensions += "-fspv-extension=SPV_GOOGLE_user_type ";

		SpvExtensions += "-fspv-extension=SPV_KHR_ray_tracing ";
		SpvExtensions += "-fspv-extension=SPV_KHR_ray_query ";
		//SpvExtensions += "-fspv-extension=SPV_KHR_shader_draw_parameters ";
		SpvExtensions += "-fspv-extension=SPV_EXT_descriptor_indexing ";
		//SpvExtensions += "-fspv-extension=SPV_EXT_shader_viewport_index_layer ";

		if (DxcPath == nullptr)
			exit(1);

		// ${DXC} -E main -DPIXEL_SHADER -spirv -T ps_6_3 -fspv-reflect -fspv-target-env=vulkan1.2 -Zi Tonemap.hlsl
		CommandLine = std::format("{} {} -E {} -T {} {} -fspv-preserve-bindings -fspv-target-env=vulkan1.2 -spirv -Zi {} -Fo {}", DxcPath, DefinesParameters, EntryPoint, ShaderStageToDxcStage(Stage), SpvExtensions, Path, TmpOutput);
	} else
	{
		printf("Compiling as GLSL\n");

		// TODO: support for non-semantic debug data with -gVS
		CommandLine = std::format("glslangValidator {} -S {} --target-env vulkan1.2 -e {} -g -o {} {}", DefinesParameters, ShaderStageToGlslangStage(Stage), EntryPoint, TmpOutput, Path);
	}

	printf("Compiling shader with CLI: %s\n", CommandLine.c_str());

	if (system(CommandLine.c_str()) == 0)
	{
		CompiledShaderBytecode Result;
		Result.Stage = Stage;
		Result.EntryPoint = EntryPoint;

		std::ifstream file(TmpOutput, std::ios::binary);
		Result.Bytecode = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		std::filesystem::remove(TmpOutput);

		return Result;
	}

	printf("Fatal error: couldn't compile shader %s\n", Path.c_str());
	exit(1);
	return {};
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		printf("Usage: ShaderCompiler [output] [input] [stages, comma-separated, with entrypoints]\n");
		printf("Stage syntax: entry1:stage1,entry2:stage2\n");
		printf("Supported stages: vs, ps, gs, hs, ds, cs, rgen, rint, rahit, rchit, rmiss\n");
		return 1;
	}

	std::string OutputPath = argv[1];
	std::filesystem::path InputPath = argv[2];
	std::string Stages = argv[3];

	std::filesystem::path InputFilename = InputPath.filename();

	std::vector<std::string> StagesDivided = DivideStringBy(Stages, ",");

	CompiledShaderData Data;
	Data.Name = InputFilename.string();

	printf("I am a shader compiler, generating %s, from %s with stages %s\n", argv[1], argv[2], argv[3]);

	for (const auto& StageStringFormatted : StagesDivided)
	{
		std::vector<std::string> EntryStageDivided = DivideStringBy(StageStringFormatted, ":");
		if (EntryStageDivided.size() != 2)
		{
			printf("Invalid stage syntax: %s\n, required format: [ENTRY]:[STAGE]", StageStringFormatted.c_str());
			return 1;
		}

		const auto& EntryPoint = EntryStageDivided[0];
		const auto& StageString = EntryStageDivided[1];

		ShaderType Stage = StringToShaderType(StageString);

		std::vector<std::string> Defines;
		Defines.push_back(DefineForShaderStage(Stage));

		Data.Bytecodes.push_back(CompileStage(InputPath.string(), OutputPath + ".tmp", EntryPoint, Defines, Stage));
	}

	printf("Compiled shader %s, it contains %i stages\n", InputFilename.string().c_str(), (int)StagesDivided.size());
	SaveCompiledShaderData(Data, OutputPath);

	return 0;
}