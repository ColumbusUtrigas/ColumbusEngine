#pragma once

#include <string>
#include <unordered_map>
#include <functional>

class CommandLine
{
public:
	void Parse(int argc, char** argv);

	bool HasArg(const std::string& key) const;
	std::string GetArgValue(const std::string& key, const std::string& defaultVal = "") const;

	void PrintHelp() const;
	void RegisterGlobalArg(const std::string& name, const std::string& description);
	void RegisterCommandlet(const std::string& name, const std::string& description, std::function<void()> func);
	void RunCommandletIfSpecified();

private:
	std::string ProcessName;
	std::unordered_map<std::string, std::string> Arguments;
	std::unordered_map<std::string, std::string> GlobalArgDescriptions;
	std::unordered_map<std::string, std::pair<std::string, std::function<void()>>> Commandlets;
};

extern CommandLine GCommandLine;
