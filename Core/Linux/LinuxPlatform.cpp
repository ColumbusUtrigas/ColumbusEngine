#include "Core/Platform.h"
#include <sys/types.h>
#include <sys/ptrace.h>

namespace Columbus
{

	bool IsRunningWithDebugger()
	{
		static int underDebugger = 0;

		static bool isCheckedAlready = false;
		if (!isCheckedAlready)
		{
			if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0)
				underDebugger = 1;
			else ptrace(PTRACE_DETACH, 0, 1, 0);

			isCheckedAlready = true;
		}

		return underDebugger == 1;
	}

}
