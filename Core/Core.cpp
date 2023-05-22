#include "Core.h"
#include <csignal>

void CrashHandler(int signal)
{
	WriteStacktraceToLog();
	exit(1);
}

void InitializeEngine()
{
	std::signal(SIGSEGV, CrashHandler);
}
