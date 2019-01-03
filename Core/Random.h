#pragma once

#include <cstdlib>
#include <ctime>

namespace Columbus
{

	struct Random
	{
		Random() { srand((unsigned int)time(NULL)); }

		template <typename Type>
		static Type Range(const Type Min, const Type Max)
		{
			return (Type)((double)rand() / RAND_MAX * (Max - Min) + Min);
		}
	};

	static Random Rnd;

}
