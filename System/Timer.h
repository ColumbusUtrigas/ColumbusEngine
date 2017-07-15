#include <chrono>

namespace C
{

	class C_Timer
	{
	private:
		std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	public:
		C_Timer();
		
		void reset();
		
		float elapsed();
		
		~C_Timer();
	};

}












