#include "Reflection.h"

namespace Reflection
{

	const EnumField* Enum::FindFieldByValue(int Value) const
	{
		for (int i = 0; i < Fields.size(); i++)
		{
			if (Fields[i].Value == Value)
				return &Fields[i];
		}

		return nullptr;
	}
}