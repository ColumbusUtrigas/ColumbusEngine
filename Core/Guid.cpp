#include "Guid.h"

#include <random>

namespace Columbus
{

	HGuid::HGuid() : Guid(0)
	{
		static std::random_device rd;
		static std::mt19937_64 gen(rd());
		static std::uniform_int_distribution<u64> dis(1, ULLONG_MAX);
		Guid = dis(gen);
	}

}
