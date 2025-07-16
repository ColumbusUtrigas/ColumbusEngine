#include "Core.h"
#include "Reflection.h"
#include "Asset.h"

#include <csignal>
#include <filesystem>
#include <thread>

using namespace Columbus;

u64 GFrameNumber = 0;

void InitializeEngine(int argc, char** argv)
{
	SetupSystemCrashHandler();
	Log::Initialization("Crash Handler was set up");
	Log::Initialization("Current working directory: %s", std::filesystem::current_path().c_str());

	Reflection::Initialise();
	Log::Initialization("Reflection initialised");

	{
		// Register args
		GCommandLine.RegisterGlobalArg("-waitforattach", "Wait for debugger to attach before continuing");
		GCommandLine.RegisterGlobalArg("-commandlet", "Run a specific commandlet");
		GCommandLine.RegisterGlobalArg("-project", "Path to the game project root folder");
		GCommandLine.RegisterGlobalArg("-assets", "Path to assets directory");
		GCommandLine.RegisterGlobalArg("--help / -h", "Prints help");

		// Register commandlets
		GCommandLine.RegisterCommandlet("BuildAssets", "Processes and builds all game assets", []() {
			printf("Running asset build...\n");
		});

		GCommandLine.RegisterCommandlet("DumpInfo", "Prints some debug info", []() {
			printf("Dumping engine info...\n");
		});

		// TODO: reflection system should automatically register all commandlets
	}

	GCommandLine.Parse(argc, argv);
	Log::Initialization("Command line arguments parsed");

	if (GCommandLine.HasArg("-waitforattach"))
	{
		Log::Initialization("Waiting for debugger to attach...");
		while (!IsDebuggerPresent())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		DEBUGBREAK();
	}
}

std::string LoadShaderFile(const std::string& Name)
{
	auto Path = "./Data/Shaders/" + Name;
	return File::ReadAllText(Path.c_str());
}
