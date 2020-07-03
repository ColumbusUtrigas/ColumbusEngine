#pragma once

#include <cstdlib>
#include <ctime>

namespace Columbus
{

	struct Random
	{
		Random() { srand((unsigned int)time(NULL)); }

		/// Generates random number in range [Min, Max]
		template <typename Type>
		static Type Range(const Type Min, const Type Max)
		{
			return ((Type)rand() / RAND_MAX * (Max - Min) + Min);
		}
	};

	static Random Rnd;

}
