#pragma once

#include "Core.h"

// TODO: cvar flags (readonly)
// TODO: variable history with links
// TODO: command interpretation
// TODO: command history
// TODO: thread safety

// usage:
// define cvar with default value
// ConsoleVariable<bool> flag("flag.name", "Description", false);
//
// obtain cvar
// ConsoleVariableSystem::GetConsoleVariable("flag.name");

enum class CVarDataType
{
	Bool,
	Int,
	Float,
};

struct CVarData
{
	bool  Bool;
	int   Int;
	float Float;

	CVarDataType Type;

	std::string Name;
	std::string Description;

	template <typename T>
	T GetValue() const
	{
		// TODO: thread safety
		switch (Type)
		{
		case CVarDataType::Bool:  return (T)Bool;
		case CVarDataType::Int:   return (T)Int;
		case CVarDataType::Float: return (T)Float;
		default:
			assert(false);
		}
	}

	template <typename T>
	void SetValue(T Value)
	{
		// TODO: thread safety
		switch (Type)
		{
		case CVarDataType::Bool:  Bool  = (T)Value;
		case CVarDataType::Int:   Int   = (T)Value;
		case CVarDataType::Float: Float = (T)Value;
		default:
			assert(false);
		}
	}
};

template <typename T>
struct ConsoleVariable
{
	ConsoleVariable() = delete;
	ConsoleVariable(ConsoleVariable&) = delete;
	ConsoleVariable(const ConsoleVariable&) = delete;

	ConsoleVariable(const char* Name, const char* Description, T DefaultValue);

	T GetValue() const
	{
		return Data.GetValue<T>();
	}

	void SetValue(T Value)
	{
		Data.SetValue<T>(Value);
	}

	const std::string& GetName() const
	{
		return Data.Name;
	}

	const std::string& GetDescription() const
	{
		return Data.Description;
	}
private:
	CVarData& Data;
};

namespace ConsoleVariableSystem
{
	CVarData* GetConsoleVariable(const char* Name);
}
