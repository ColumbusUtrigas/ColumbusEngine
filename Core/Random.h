#pragma once

#include <random>
#include <ctime>

namespace Columbus
{
	namespace
	{
		std::mt19937 Rnd(static_cast<unsigned int>(time(NULL)));
	}

	struct Random
	{
		template <typename Type>
		static Type Range(const Type Min, const Type Max)
		{
			std::uniform_real_distribution<Type> Generator(Min, Max);

			return Generator(Rnd);
		}
	};

}
