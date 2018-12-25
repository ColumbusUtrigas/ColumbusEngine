#include <System/System.h>

namespace Columbus
{

	const char* GetSystem()
	{
		#ifdef COLUMBUS_SYSTEM_WINDOWS
			return "Windows";
		#endif

		#ifdef COLUMBUS_SYSTEM_LINUX
			return "Linux";
		#endif

		#ifdef COLUMBUS_SYSTEM_APPLE
			return "Apple";
		#endif

		return "Unknown";
	}
	
	bool CheckWindows()
	{
		if (GetSystem() == "Windows")
		{
			return true;
		}

		return false;
	}

}


