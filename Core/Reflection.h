#pragma once

#include <vector>

// code generation macros, used only by header tool
// for regular c++ compiler they don't produce anything

#define CENUM()
#define CSTRUCT()
#define CFIELD()
#define CFUNC()

namespace Reflection
{

	struct EnumField
	{
		const char* Name;
		int Value;
		int Index;
	};

	struct Enum
	{
		const char* Name;
		std::vector<EnumField> Fields;

		const EnumField* FindFieldByValue(int Value) const;
	};

	// meant to be specialised for every enum
	template <typename T>
	const Enum* FindEnum()
	{
		return nullptr;
	}

}
