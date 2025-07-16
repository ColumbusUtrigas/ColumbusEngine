#include "Core.h"
#include "String.h"
#include "CommandLine.h"

#include <filesystem>

using namespace Columbus;

CommandLine GCommandLine;

void CommandLine::Parse(int argc, char** argv)
{
	std::filesystem::path exePath(argv[0]);
	ProcessName = exePath.filename().string(); // just the name, without extension;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg.rfind("-", 0) == 0)
		{
			size_t eq = arg.find('=');
			if (eq != std::string::npos)
			{
				Arguments[arg.substr(0, eq)] = arg.substr(eq + 1);
			}
			else
			{
				Arguments[arg] = "true";
			}
		}
	}

	if (HasArg("--help") || HasArg("-h"))
		PrintHelp();
}

bool CommandLine::HasArg(const std::string& key) const
{
	return Arguments.find(str_tolower(key)) != Arguments.end();
}

std::string CommandLine::GetArgValue(const std::string& key, const std::string& defaultVal) const
{
	auto it = Arguments.find(str_tolower(key));
	return it != Arguments.end() ? it->second : defaultVal;
}

void CommandLine::RegisterGlobalArg(const std::string& name, const std::string& description)
{
	GlobalArgDescriptions[name] = description;
}

void CommandLine::RegisterCommandlet(const std::string& name, const std::string& description, std::function<void()> func)
{
	Commandlets[name] = { description, func };
}

void CommandLine::RunCommandletIfSpecified()
{
	if (HasArg("-commandlet"))
	{
		std::string name = GetArgValue("-commandlet");
		auto it = Commandlets.find(name);
		if (it != Commandlets.end())
		{
			it->second.second(); // Run the commandlet
		}
		else
		{
			printf("Commandlet '%s' not found.\n", name.c_str());	
			PrintHelp();
		}
	}
}

void CommandLine::PrintHelp() const
{
	printf("Usage: %s [options]\n\n", ProcessName.c_str());

	printf("Global Arguments:\n");
	for (const auto& pair : GlobalArgDescriptions)
		printf("  %s\t%s\n", pair.first.c_str(), pair.second.c_str());

	printf("\nAvailable Commandlets:\n");
	for (const auto& pair : Commandlets)
		printf("  %s\t%s\n", pair.first.c_str(), pair.second.first.c_str());

	exit(0);
}
