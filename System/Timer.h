/************************************************
*                     Timer.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <chrono>

namespace Columbus
{

	class C_Timer
	{
	private:
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	public:
		C_Timer();
		
		void reset();
		double elapsed();
		
		~C_Timer();
	};

}
