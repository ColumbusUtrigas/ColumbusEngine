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

	class C_Random
	{
	public:
		C_Random();

		static float range(float aMin, float aMax);

		~C_Random();
	};

}
