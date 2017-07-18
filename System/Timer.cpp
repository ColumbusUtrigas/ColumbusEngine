#include <System/Timer.h>

namespace C
{

	C_Timer::C_Timer()
	{
	
	}

	void C_Timer::reset()
	{
		start = std::chrono::high_resolution_clock::now();
		end = std::chrono::high_resolution_clock::now();
	}
	
	float C_Timer::elapsed()
	{
		end = std::chrono::high_resolution_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		return dur.count();
	}

	C_Timer::~C_Timer()
	{
	
	}

}










