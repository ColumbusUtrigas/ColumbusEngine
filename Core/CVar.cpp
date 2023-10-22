#include "CVar.h"

#include <mutex>
#include <string>
#include <unordered_map>

static std::unordered_map<std::string, CVarData>& GetCvarMap()
{
	// TODO: thread safety
	static std::unordered_map<std::string, CVarData> CvarMap;
	return CvarMap;
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
}