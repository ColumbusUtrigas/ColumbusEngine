#pragma once

#include <stdlib.h>

namespace Columbus
{
	class Random
	{
	public:
		/// Generates a random value in range Min...Max
		inline static float Range(const float Min, const float Max)
		{
			return Min + (Max - Min) * (rand() / static_cast<float>(RAND_MAX));
		}
	};
}
