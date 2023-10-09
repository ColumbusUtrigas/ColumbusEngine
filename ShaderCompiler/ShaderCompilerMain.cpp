#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <format>
#include <fstream>

// TODO: get it from the engine header, remove duplication
enum class ShaderType
{
	Vertex = 1,
	Pixel = 2,
	Hull = 4,
	Domain = 8,
	Geometry = 16,
	AllGraphics = Vertex | Pixel | Hull | Domain | Geometry,

	Compute = 32,

	Raygen = 64,
	Miss = 128,
	Anyhit = 256,
	ClosestHit = 512,
	Intersection = 1024,
	AllRayTracing = Raygen | Miss | Anyhit | ClosestHit | Intersection,
};

// TODO: better data layout?
struct CompiledShaderBytecode
{
	ShaderType Stage;
	std::string EntryPoint;
	std::vector<uint8_t> Bytecode;
	// reflection data
};

struct CompiledShaderData
{
	std::string Name;
	std::vector<CompiledShaderBytecode> Bytecodes;
	// reflection data
};

struct CompiledShaderDataHeader
{
	uint32_t MagicNumber; // CCSD = 0x44534343 LE
	uint32_t Version;
	uint32_t NameLength;
	uint32_t NumBytecodes;
	// Flags, Permutations, Reflection
};

struct CompiledShaderBytecodeHeader
{
	uint32_t Stage; // ShaderType 
	uint32_t EntryPointNameLength;
	uint32_t BytecodeLength;
	uint32_t _Unused;
};

void SaveCompiledShaderData(const CompiledShaderData& Data, const std::string& Path)
{
	CompiledShaderDataHeader DataHeader {
		.MagicNumber  = 0x44534343,
		.Version      = 1,
		.NameLength   = (uint32_t)Data.Name.size(),
		.NumBytecodes = (uint32_t)Data.Bytecodes.size(),
	};

	FILE* f = fopen(Path.c_str(), "wb");

	fwrite(&DataHeader, sizeof(DataHeader), 1, f);
	fwrite(Data.Name.c_str(), sizeof(char), DataHeader.NameLength, f);

	for (const auto& Bytecode : Data.Bytecodes)
	{
		CompiledShaderBytecodeHeader BytecodeHeader{
			.Stage                = (uint32_t)Bytecode.Stage,
			.EntryPointNameLength = (uint32_t)Bytecode.EntryPoint.size(),
			.BytecodeLength       = (uint32_t)Bytecode.Bytecode.size(),
			._Unused              = 0
		};

		fwrite(&BytecodeHeader, sizeof(BytecodeHeader), 1, f);
		fwrite(Bytecode.EntryPoint.c_str(), sizeof(char), Bytecode.EntryPoint.size(), f);
		fwrite(Bytecode.Bytecode.data(), sizeof(uint8_t), Bytecode.Bytecode.size(), f);
	}

	fclose(f);
}

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
		case ShaderType::Pixel:	       return  "PIXEL_SHADER";
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

std::vector<std::string> DivideStringBy(std::string s, std::string delimiter)
{
	std::vector<std::string> result;

	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		result.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	result.push_back(s);

	return result;
}

CompiledShaderBytecode CompileStage(const std::string& Path, const std::string& EntryPoint, const std::vector<std::string>& Defines, ShaderType Stage)
{
	std::string DefinesParameters;
	for (const auto& Define : Defines)
	{
		DefinesParameters += " -D" + Define;
	}

	char OutputFileName[2048]{0};
	std::tmpnam(OutputFileName);

	std::string CommandLine = std::format("glslangValidator {} -S {} --target-env vulkan1.2 -e {} -g -gVS -o {} {}", DefinesParameters, ShaderStageToGlslangStage(Stage), EntryPoint, OutputFileName, Path);

	printf("Compiling shader with CLI: %s\n", CommandLine.c_str());

	if (system(CommandLine.c_str()) == 0)
	{
		CompiledShaderBytecode Result;
		Result.Stage = Stage;
		Result.EntryPoint = EntryPoint;

		std::ifstream file(OutputFileName, std::ios::binary);
		Result.Bytecode = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		return Result;
	}

	printf("Fatal error: couldn't compile shader %s\n", Path.c_str());
	exit(1);
	return {};
}

int main(int argc, char** argv)
{
	std::string OutputPath = argv[1];
	std::filesystem::path InputPath = argv[2];
	std::string Stages = argv[3];

	std::filesystem::path InputFilename = InputPath.filename();

	std::vector<std::string> StagesDivided = DivideStringBy(Stages, ",");

	CompiledShaderData Data;
	Data.Name = InputFilename.string();

	printf("I am a shader compiler, generating %s, from %s with stages %s\n", argv[1], argv[2], argv[3]);

	for (const auto& StageString : StagesDivided)
	{
		ShaderType Stage = StringToShaderType(StageString);

		std::vector<std::string> Defines;
		Defines.push_back(DefineForShaderStage(Stage));

		Data.Bytecodes.push_back(CompileStage(InputPath.string(), "main", Defines, Stage));
	}

	printf("Compiled shader %s, it contains %i stages\n", InputFilename.string().c_str(), (int)StagesDivided.size());
	SaveCompiledShaderData(Data, OutputPath);

	return 0;
}