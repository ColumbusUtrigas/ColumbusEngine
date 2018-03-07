/************************************************
*                   Random.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   31.10.2017                  *
*************************************************/

#pragma once

#include <random>
#include <cstdlib>
#include <ctime>

namespace Columbus
{

	class Random
	{
	public:
		Random();

		static float range(float aMin, float aMax);

		~Random();
	};

}
