#include "Core.h"
#include "Reflection.h"
#include "Asset.h"

#include <csignal>
#include <filesystem>

using namespace Columbus;

u64 GFrameNumber = 0;

void InitializeEngine()
{
	SetupSystemCrashHandler();

	Log::Initialization("Crash Handler was set up");
	Log::Initialization("Current working directory: %s", std::filesystem::current_path().c_str());

	Reflection::Initialise();
	Log::Initialization("Reflection initialised");
}

std::string LoadShaderFile(const std::string& Name)
{
	auto Path = "./Data/Shaders/" + Name;
	return File::ReadAllText(Path.c_str());
}
