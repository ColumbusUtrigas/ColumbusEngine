/************************************************
*                     Timer.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <chrono>

namespace C
{

	class C_Timer
	{
	private:
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	public:
		//Constructor
		C_Timer();
		//Reset timer
		void reset();
		//Return elapsed time
		double elapsed();
		//Destructor
		~C_Timer();
	};

}
