#include "CVar.h"
#include "Profiling/Profiling.h"

#include <algorithm>
#include <mutex>
#include <string>
#include <unordered_map>
#include <stdio.h>

static std::unordered_map<std::string, CVarData>& GetCvarMap()
{
	// TODO: thread safety
	static std::unordered_map<std::string, CVarData> CvarMap;
	return CvarMap;
}

static std::vector<const char*>& GetCvarList()
{
	static std::vector<const char*> List;
	return List;
}

static void AddCvarToList(const char* Name)
{
	GetCvarList().push_back(Name);
	std::sort(GetCvarList().begin(), GetCvarList().end());
}

CVarData& GetOrCreateCvarData(const char* Name, const char* Description, bool Bool, int Int, float Float, CVarDataType Type)
{
	// TODO: thread safety
	auto& CvarMap = GetCvarMap();
	if (!CvarMap.contains(Name))
	{
		CvarMap[Name] = CVarData {
			.Bool = Bool,
			.Int = Int,
			.Float = Float,
			.Type = Type,
			.Name = Name,
			.Description = Description
		};

		AddCvarToList(Name);
	}
	else
	{
		Columbus::Log::Fatal("Duplicate initialisation of CVar: %s", Name);
	}

	return CvarMap[Name];
}

template <>
ConsoleVariable<bool>::ConsoleVariable(const char* Name, const char* Description, bool DefaultValue) :
	Data(GetOrCreateCvarData(Name, Description, DefaultValue, 0, 0, CVarDataType::Bool)) {}

template <>
ConsoleVariable<int>::ConsoleVariable(const char* Name, const char* Description, int DefaultValue) :
	Data(GetOrCreateCvarData(Name, Description, false, DefaultValue, 0, CVarDataType::Int)) {}

template <>
ConsoleVariable<float>::ConsoleVariable(const char* Name, const char* Description, float DefaultValue) :
	Data(GetOrCreateCvarData(Name, Description, false, 0, DefaultValue, CVarDataType::Float)) {}

namespace ConsoleVariableSystem
{
	CVarData* GetConsoleVariable(const char* Name)
	{
		// TODO: thread safety
		auto& CvarMap = GetCvarMap();
		if (!CvarMap.contains(Name))
			return nullptr;
		return &CvarMap[Name];
	}

#define ConsoleCommandOutput(Fmt, ...) int size = snprintf(NULL, 0, Fmt, __VA_ARGS__); Result = new char[size+1]; snprintf(Result, size+1, Fmt, __VA_ARGS__); Result[size] = 0;
#define ConsoleCommandMessage(Fmt, ...) Columbus::Log::Message(Fmt, __VA_ARGS__); ConsoleCommandOutput(Fmt, __VA_ARGS__);
#define ConsoleCommandError(Fmt, ...) Columbus::Log::Error(Fmt, __VA_ARGS__); ConsoleCommandOutput(Fmt, __VA_ARGS__);

	static char* StatHelp()
	{
		std::string Result = "Available stats:";
		for (const char* Category : Columbus::GetProfilerCategoryListCPU())
		{
			Result += std::string("\n") + Category;
		}

		Columbus::Log::Message("%s", Result.c_str());

		return strdup(Result.c_str());
	}

	char* RunConsoleCommand(const char* Command)
	{
		Columbus::Log::Message(Command);

		char Cmd[256]{ 0 };
		char Value[256]{ 0 };
		int ScanResult = sscanf(Command, "%s %s", Cmd, Value);

		char* Result = nullptr;

		if (ScanResult == 1) // get command
		{
			if (strcmp(Cmd, "stat") == 0)
			{
				Result = StatHelp();
			}
			else if (strcmp(Cmd, "list") == 0)
			{
				std::string Str = "Listing available CVars:";
				for (const char* A : GetCvarList())
				{
					Str += std::string("\n") + A;
				}

				Columbus::Log::Message("%s", Str.c_str());

				Result = strdup(Str.c_str());
			}
			else if (strcmp(Cmd, "help") == 0 || strcmp(Cmd, "?") == 0)
			{
				ConsoleCommandMessage("%s", "Available commands: list - prints all CVars, help or ? - this message, [cvar] - get value of a CVar"
				                       "[cvar] ? - get description of a cvar, [cvar] [value] - set new value to CVar, "
				                       "stat or stat ? - print available stats, stat [category] - toggle stat");
			}
			else
			{
				CVarData* Cvar = GetConsoleVariable(Cmd);
				if (Cvar == nullptr)
				{
					ConsoleCommandError("Unknown command: %s", Cmd);
				}
				else
				{
					if (Cvar->Type == CVarDataType::Float)
					{
						ConsoleCommandMessage("%s=%f", Cmd, Cvar->Float);
					}
					else
					{
						ConsoleCommandMessage("%s=%i", Cmd, Cvar->GetValue<int>());
					}
				}
			}
		}
		else if (ScanResult == 2) // set/describe command
		{
			if (strcmp(Cmd, "stat") == 0)
			{
				if (strcmp(Value, "?") == 0)
				{
					Result = StatHelp();
				} else
				{
					// TODO:
				}
			}
			else
			{
				CVarData* Cvar = GetConsoleVariable(Cmd);
				if (Cvar == nullptr)
				{
					ConsoleCommandError("Unknown command: %s", Cmd);
				}
				else
				{
					char* EndInt;
					char* EndFloat;
					int   Int   = strtol(Value, &EndInt, 10);
					float Float = strtof(Value, &EndFloat);

					if (EndInt != Value && EndFloat <= EndInt)
					{
						Cvar->SetValue(Int);
						ConsoleCommandMessage("%s=%i", Cmd, Int);
					} else if (EndFloat != Value)
					{
						Cvar->SetValue(Float);
						ConsoleCommandMessage("%s=%f", Cmd, Float);
					} else if (strcmp(Value, "?") == 0)
					{
						ConsoleCommandMessage("%s", Cvar->Description.c_str());
					} else
					{
						ConsoleCommandError("Unable to parse value %s", Value);
					}
				}
			}
		}

		return Result;
	}
}