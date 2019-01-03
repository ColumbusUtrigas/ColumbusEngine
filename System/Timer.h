#pragma once

#include <Core/Platform/Platform.h>

#ifdef COLUMBUS_PLATFORM_WINDOWS
	#include <SDL.h>
#else
	#include <sys/time.h>
#endif

namespace Columbus
{

#ifdef COLUMBUS_PLATFORM_WINDOWS
	class Timer
	{
	private:
		Uint64 Frequency, Start;
	public:
		Timer() : Frequency(SDL_GetPerformanceFrequency())
		{
			Start = SDL_GetPerformanceCounter();
		}
		
		void Reset()
		{
			Start = SDL_GetPerformanceCounter();
		}

		double Elapsed()
		{
			Uint64 End = SDL_GetPerformanceCounter();
			return (double)(End - Start) / (double)(Frequency);
		}
		
		~Timer() {}
	};
#else
	class Timer
	{
	private:
		struct timeval Start, End;
	public:
		Timer()
		{
			gettimeofday(&Start, nullptr);
		}

		void Reset()
		{
			gettimeofday(&Start, nullptr);
		}

		double Elapsed()
		{
			gettimeofday(&End, nullptr);
			long long Sec = End.tv_sec - Start.tv_sec;
			long long Usec = End.tv_usec - Start.tv_usec;
			return (double)(Sec) + (double)(Usec) / 1000000.0;
		}

		~Timer() {}
	};
#endif

}


