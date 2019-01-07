#pragma once

#include <Core/Platform/Platform.h>

#ifdef COLUMBUS_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

namespace Columbus
{

#ifdef COLUMBUS_PLATFORM_WINDOWS
	class Timer
	{
	private:
		LARGE_INTEGER Frequency, Start;
	public:
		Timer()
		{
			QueryPerformanceFrequency(&Frequency);
			QueryPerformanceCounter(&Start);
		}
		
		void Reset()
		{
			QueryPerformanceCounter(&Start);
		}

		double Elapsed()
		{
			LARGE_INTEGER End;
			QueryPerformanceCounter(&End);
			return (double)(End.QuadPart - Start.QuadPart) / (double)(Frequency.QuadPart);
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


