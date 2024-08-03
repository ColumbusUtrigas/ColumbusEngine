#include "Core/Platform.h"
#include <windows.h>

namespace Columbus
{

	bool IsRunningWithDebugger()
	{
		return IsDebuggerPresent() == TRUE;
	}

}