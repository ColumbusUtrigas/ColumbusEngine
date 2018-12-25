#pragma once

#include <chrono>

namespace Columbus
{

	class Timer
	{
	private:
		std::chrono::steady_clock::time_point Start = std::chrono::steady_clock::now();
	public:
		Timer() {}
		
		void Reset()
		{
			Start = std::chrono::steady_clock::now();
		}

		double Elapsed()
		{
			auto End = std::chrono::steady_clock::now();
			auto Time = End - Start;
			return double(Time.count()) * std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;
		}
		
		~Timer() {}
	};

}


