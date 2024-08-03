#include "Core.h"
#include <csignal>
#include <filesystem>

using namespace Columbus;

u64 GFrameNumber = 0;

void CrashHandler(int signal)
{
	WriteStacktraceToLog();
	exit(1);
}

void InitializeEngine()
{
	std::signal(SIGSEGV, CrashHandler);
	Log::Initialization("Crash Handler was set up");
	Log::Initialization("Current working directory: %s", std::filesystem::current_path().c_str());
}

std::string LoadShaderFile(const std::string& Name)
{
	auto Path = "./Data/Shaders/" + Name;
	return File::ReadAllText(Path.c_str());
}
