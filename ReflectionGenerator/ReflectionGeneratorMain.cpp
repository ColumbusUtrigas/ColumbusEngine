// third party
#include "../Lib/json/single_include/nlohmann/json.hpp"

// std
#include <regex>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <ranges>
#include <format>
#include <string_view>
#include <stdio.h>


struct EnumField
{
	std::string Name;
	int Value;
};

struct Enum
{
	std::string Name;
	std::string Meta;
	std::vector<EnumField> Fields;
};


struct FileData
{
	std::filesystem::path SourcePath;
	std::filesystem::path GenHeaderPath;

	bool bWorthGenerating = false;

	std::vector<Enum> Enums;
	// TODO: structs
};

int GProcessedHeaderFiles = 0;
int GGeneratedHeaderFiles = 0;
int GGeneratedSourceFiles = 0;
int GTotalEnums = 0;

std::regex Re_Enum;

void InitialiseRegex()
{
	Re_Enum = R"(CENUM\((.*?)\)\s*enum\s+class\s+(.*?)\s*\{([\s\S]*?)\};)";
}

std::string_view trim(std::string_view sv)
{
	const char* trimChars = " \t\n\r";
	sv.remove_prefix(std::min(sv.find_first_not_of(trimChars), sv.size()));
	sv.remove_suffix(sv.size() - sv.find_last_not_of(trimChars) - 1);
	return sv;
}

// given content between brackets of the enum, extract all needed information
void ProcessHeaderEnum(Enum& En, std::string_view content_view)
{
	// separate enumerations
	auto comma_split = content_view
		| std::ranges::views::split(',')
		| std::ranges::views::transform([](auto&& str) { return std::string_view(&*str.begin(), std::ranges::distance(str)); });

	int current_value = 0;
	for (auto&& word : comma_split)
	{
		auto trimmed = trim(word);

		if (trimmed.empty()) continue;

		// if there is A=3 in the enumeration
		auto equal_split = trimmed
			| std::ranges::views::split('=')
			| std::ranges::views::transform([](auto&& str) { return std::string_view(&*str.begin(), std::ranges::distance(str)); });

		std::string parts[2]{};

		int split_part = 0;
		for (auto&& part : equal_split)
		{
			if (split_part > 1) break;
			parts[split_part++] = trim(part);
		}

		if (!parts[1].empty())
			current_value = std::stoi(parts[1]);

		EnumField Field{
			.Name = parts[0],
			.Value = current_value
		};
		current_value++;

		En.Fields.push_back(Field);
	}
}

FileData ProcessHeaderFile(const std::filesystem::path& path)
{
	// read whole file
	std::ifstream ifs(path);
	std::string file_contents((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

	FileData Result;
	Result.SourcePath = path;

	// enums
	{
		std::string contents = file_contents;
		std::smatch res;
		while (std::regex_search(contents, res, Re_Enum))
		{
			Enum En;
			En.Meta = res[1];
			En.Name = res[2];

			// everything between brackets
			std::string_view content_view{contents.data() + (size_t)res.position(3), (size_t)res.length(3)};

			ProcessHeaderEnum(En, content_view);

			Result.Enums.push_back(En);
			Result.bWorthGenerating = true;

			GTotalEnums++;

			contents = res.suffix();
		}
	}

	GProcessedHeaderFiles++;
	return Result;
}

std::filesystem::path CreateGeneratedFilePath(
	std::filesystem::path source_path,
	std::filesystem::path input_root,
	std::filesystem::path write_root,
	const char* wanted_extension)
{
	using namespace std::filesystem;

	auto parent_path = source_path.parent_path();
	auto relative_to_file = relative(parent_path, input_root);

	// recreate the same file structure as source for generated files
	auto gen_root = write_root / relative_to_file;
	auto gen_path = gen_root / (source_path.filename());
	create_directories(gen_root);
	gen_path.replace_extension(path(wanted_extension));

	return gen_path;
}

void WriteGeneratedHeader(FileData& File, std::filesystem::path input_root, std::filesystem::path write_root)
{
	if (!File.bWorthGenerating)
		return;

	auto gen_path = CreateGeneratedFilePath(File.SourcePath, input_root, write_root, ".gen.h");
	File.GenHeaderPath = gen_path;
	
	std::ofstream ofs(gen_path);
	ofs << "// Automatically generated file, do not edit manually.\n";
	ofs << "#pragma once\n\n";
	ofs << "#include <Core/Reflection.h>\n\n";

	ofs << "// enum forward declarations\n";
	for (const auto& En : File.Enums)
	{
		ofs << std::format("enum class {};\n", En.Name);
	}

	ofs << "\n";
	ofs << "// enum functions\n";
	for (const auto& En : File.Enums)
	{
		ofs << std::format("template <> const Reflection::Enum* Reflection::FindEnum<{}>();\n", En.Name);
	}

	GGeneratedHeaderFiles++;
}

void WriteGeneratedSource(const FileData& File, std::filesystem::path input_root, std::filesystem::path write_root)
{
	if (!File.bWorthGenerating)
		return;

	using namespace std::filesystem;

	auto gen_path = CreateGeneratedFilePath(File.SourcePath, input_root, write_root, ".gen.cpp");

	std::ofstream ofs(gen_path);
	ofs << "// Automatically generated file, do not edit manually.\n";
	ofs << std::format("#include \"{}\"\n\n", relative(File.GenHeaderPath, gen_path.parent_path()).string());

	for (const auto& En : File.Enums)
	{
		auto reg_struct_name = std::format("{}__Registration", En.Name);
		auto reg_struct_instance = std::format("{}__Instance", reg_struct_name);

		ofs << std::format("struct {}\n", reg_struct_name);
		ofs << "{\n";
		ofs << "\tReflection::Enum EnumData;\n\n";
		ofs << std::format("\t{}()\n", reg_struct_name);
		ofs << "\t{\n";
		ofs << std::format("\t\tEnumData.Name = \"{}\";\n", En.Name);
		ofs << std::format("\t\tEnumData.Fields.reserve({});\n", En.Fields.size());

		int Index = 0;
		for (const auto& Field : En.Fields)
		{
			ofs << std::format(
				"\t\tEnumData.Fields.push_back(Reflection::EnumField{{ .Name = \"{}\", .Value = (int){}::{}, .Index = {} }});\n",
				Field.Name, En.Name, Field.Name, Index
			);
			Index++;
		}

		ofs << "\t}\n";
		ofs << std::format("}} {};\n\n", reg_struct_instance);

		ofs << std::format("template <> const Reflection::Enum* Reflection::FindEnum<{}>()\n", En.Name);
		ofs << "{\n";
		ofs << std::format("\treturn &{}.EnumData;\n", reg_struct_instance);
		ofs << "}\n";
	}

	GGeneratedSourceFiles++;
}

// CLI: <ReflectionGeneratorBinary> <Root folder> <Output folder>
int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("ERROR: ReflectionGenerator - not enough arguments\n");
		return 1;
	}

	nlohmann::json json;
	{
		auto codegen_path = std::filesystem::path(argv[1]) / "Codegen.json";
		std::ifstream ifs(codegen_path);
		if (!ifs.is_open())
		{
			printf("ERROR: ReflectionGenerator - couldn't find Codegen.json\n");
			return 2;
		}

		ifs >> json;
	}

	InitialiseRegex();

	auto input_directory = std::filesystem::path(argv[1]);
	auto output_directory = std::filesystem::path(argv[2]);
	std::filesystem::create_directory(output_directory);

	for (const auto& folder : json["Folders"])
	{
		using recursive_dir_iter = std::filesystem::recursive_directory_iterator;

		auto folder_path = std::filesystem::path(argv[1]) / std::string(folder);

		for (const auto& entry : recursive_dir_iter(folder_path))
		{
			const auto path = std::filesystem::path(entry);

			// filter for header files
			if (entry.is_regular_file() && path.has_extension() && path.extension() == ".h")
			{
				FileData File = ProcessHeaderFile(path);
				WriteGeneratedHeader(File, input_directory, output_directory);
				WriteGeneratedSource(File, input_directory, output_directory);
			}
		}
	}

	printf("Processed header files: %i\n", GProcessedHeaderFiles);
	printf("Generated header files: %i\n", GGeneratedHeaderFiles);
	printf("Generated source files: %i\n", GGeneratedSourceFiles);
	printf("Total enums: %i\n", GTotalEnums);

	return 0;
}