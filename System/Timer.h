#pragma once

#include <SDL.h>

namespace Columbus
{

	class Timer
	{
	private:
		Uint64 Frequency, Start;
	public:
		Timer() : Frequency(SDL_GetPerformanceFrequency()) {}
		
		void Reset()
		{
			Start = SDL_GetPerformanceCounter();
		}

		double Elapsed()
		{
			Uint64 End = SDL_GetPerformanceCounter();
			return (double)(End - Start) / Frequency;
		}
		
		~Timer() {}
	};

}


