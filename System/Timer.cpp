/************************************************
*                    Timer.cpp                  *
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
		start = std::chrono::steady_clock::now();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return elapsed timer
	double C_Timer::elapsed()
	{
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		std::chrono::steady_clock::duration time_span = end - start;
		double nseconds = double(time_span.count()) * std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;
		return nseconds;

	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Timer::~C_Timer()
	{

	}

}
