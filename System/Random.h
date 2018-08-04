#pragma once

#include <random>
#include <cstdlib>
#include <ctime>

namespace Columbus
{
	namespace
	{
		std::mt19937 Rnd(static_cast<unsigned int>(time(NULL)));
	}

	class Random
	{
	public:
		Random();

		template <typename Type>
		static Type range(const Type Min, const Type Max)
		{
			std::uniform_real_distribution<Type> Generator(Min, Max);

			return Generator(Rnd);
		}

		~Random();
	};

}
