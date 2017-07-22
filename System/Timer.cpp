/************************************************
*              			 Timer.cpp                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <System/Timer.h>

namespace C
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Timer::C_Timer()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//Reset timer
	void C_Timer::reset()
	{
		start = std::chrono::high_resolution_clock::now();
		end = std::chrono::high_resolution_clock::now();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return elapsed timer
	float C_Timer::elapsed()
	{
		end = std::chrono::high_resolution_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		return dur.count();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Timer::~C_Timer()
	{

	}

}
