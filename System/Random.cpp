/************************************************
*                  Random.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   31.10.2017                  *
*************************************************/

#include <System/Random.h>

namespace Columbus
{

	std::mt19937 rng(static_cast<unsigned int>(time(NULL)));

	//////////////////////////////////////////////////////////////////////////////
	Random::Random()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	float Random::range(float aMin, float aMax)
	{
		std::uniform_real_distribution<float> gen(aMin, aMax);

		return gen(rng);
	}
	//////////////////////////////////////////////////////////////////////////////
	Random::~Random()
	{

	}

}





