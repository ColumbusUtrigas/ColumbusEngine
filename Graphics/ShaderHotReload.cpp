#include <Graphics/ShaderHotReload.h>

#include <Core/CVar.h>
#include <Graphics/ShaderCache.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace Columbus
{
#ifndef COLUMBUS_CMAKE_COMMAND
#define COLUMBUS_CMAKE_COMMAND "cmake"
#endif

#ifndef COLUMBUS_CMAKE_BINARY_DIR
#define COLUMBUS_CMAKE_BINARY_DIR ""
#endif

#ifndef COLUMBUS_CMAKE_BUILD_CONFIG
#define COLUMBUS_CMAKE_BUILD_CONFIG "Debug"
#endif

	namespace
	{
		ConsoleVariable<bool> CVar_ShaderHotReload("r.ShaderHotReload", "Enable shader .csd timestamp checks and reloads", false);
		ConsoleVariable<int> CVar_ShaderRecompile("r.ShaderRecompile", "Set to 1 to rebuild the Shaders target and reload shader caches", 0);
		uint64_t GShaderReloadGeneration = 0;

		std::filesystem::path FindShaderBuildDirectory()
		{
			if (COLUMBUS_CMAKE_BINARY_DIR[0] != '\0')
			{
				std::filesystem::path BuildDir = COLUMBUS_CMAKE_BINARY_DIR;
				if (std::filesystem::exists(BuildDir))
				{
					return BuildDir;
				}

				Log::Warning("[ShaderHotReload] Compile-time CMake binary directory does not exist: %s", BuildDir.string().c_str());
			}

			std::filesystem::path Path = std::filesystem::current_path();
			for (;;)
			{
				std::filesystem::path BuildDir = Path / "build" / "vs2022";
				if (std::filesystem::exists(BuildDir))
				{
					return BuildDir;
				}

				if (!Path.has_parent_path() || Path == Path.parent_path())
				{
					return {};
				}

				Path = Path.parent_path();
			}
		}

		int RunCommandAndLogOutput(const std::string& Command)
		{
			Log::Message("[ShaderHotReload] Running: %s", Command.c_str());

#if defined(_WIN32)
			const std::string ShellCommand = "\"" + Command + "\" 2>&1";
			FILE* Pipe = _popen(ShellCommand.c_str(), "r");
#else
			FILE* Pipe = popen((Command + " 2>&1").c_str(), "r");
#endif

			if (Pipe == nullptr)
			{
				Log::Error("[ShaderHotReload] Failed to run shader build command");
				return -1;
			}

			std::array<char, 512> Buffer;
			while (fgets(Buffer.data(), (int)Buffer.size(), Pipe) != nullptr)
			{
				Log::Message("[ShaderHotReload] %s", Buffer.data());
			}

#if defined(_WIN32)
			return _pclose(Pipe);
#else
			return pclose(Pipe);
#endif
		}

		bool ProcessShaderRecompileRequest()
		{
			if (!CVar_ShaderHotReload.GetValue())
				return false;

			if (CVar_ShaderRecompile.GetValue() == 0)
				return false;

			CVar_ShaderRecompile.SetValue(0);

			std::filesystem::path BuildDir = FindShaderBuildDirectory();
			if (BuildDir.empty())
			{
				Log::Error("[ShaderHotReload] Could not find CMake build directory. Compile-time path: %s, current directory: %s", COLUMBUS_CMAKE_BINARY_DIR, std::filesystem::current_path().string().c_str());
				return true;
			}

			const std::string Command = "\"" COLUMBUS_CMAKE_COMMAND "\" --build \"" + BuildDir.string() + "\" --target Shaders --config " COLUMBUS_CMAKE_BUILD_CONFIG;
			const int Result = RunCommandAndLogOutput(Command);
			if (Result != 0)
			{
				Log::Error("[ShaderHotReload] Shader build failed with code %i; keeping existing pipelines", Result);
				return true;
			}

			GShaderReloadGeneration++;
			Log::Message("[ShaderHotReload] Shader build succeeded; reload generation is now %llu", (unsigned long long)GShaderReloadGeneration);
			return false;
		}

		bool GetShaderFileWriteTime(const char* Path, std::filesystem::file_time_type& OutWriteTime)
		{
			std::error_code Error;
			OutWriteTime = std::filesystem::last_write_time(Path, Error);
			if (Error)
			{
				Log::Error("[ShaderHotReload] Could not read timestamp for %s: %s", Path, Error.message().c_str());
				return false;
			}

			return true;
		}
	}

	void ShaderHotReload::Update(ShaderCache& Cache)
	{
		const bool bBuildFailed = ProcessShaderRecompileRequest();
		if (bBuildFailed)
			return;

		if (SeenReloadGeneration != GShaderReloadGeneration)
		{
			Cache.InvalidateAll();
			SeenReloadGeneration = GShaderReloadGeneration;
		}
	}

	bool ShaderHotReload::ReloadShaderDataIfNeeded(
		const char* Path,
		CompiledShaderData& PackedBytecode,
		bool& bLoadedBytecode)
	{
		const bool bBuildFailed = ProcessShaderRecompileRequest();
		ShaderHotReloadState& State = ShaderStates[Path];

		if (!bLoadedBytecode)
		{
			PackedBytecode = LoadCompiledShaderData(Path);
			bLoadedBytecode = true;
			State.bHasLastWriteTime = GetShaderFileWriteTime(Path, State.LastWriteTime);
			State.SeenReloadGeneration = GShaderReloadGeneration;
			return false;
		}

		if (!CVar_ShaderHotReload.GetValue())
			return false;

		if (bBuildFailed)
			return false;

		std::filesystem::file_time_type CurrentWriteTime;
		const bool bHasCurrentWriteTime = GetShaderFileWriteTime(Path, CurrentWriteTime);
		const bool bTimestampChanged = bHasCurrentWriteTime && (!State.bHasLastWriteTime || CurrentWriteTime != State.LastWriteTime);
		const bool bGenerationChanged = State.SeenReloadGeneration != GShaderReloadGeneration;

		if (!bTimestampChanged && !bGenerationChanged)
			return false;

		PackedBytecode = LoadCompiledShaderData(Path);
		State.LastWriteTime = CurrentWriteTime;
		State.bHasLastWriteTime = bHasCurrentWriteTime;
		State.SeenReloadGeneration = GShaderReloadGeneration;

		Log::Message("[ShaderHotReload] Reloaded %s", Path);
		return true;
	}
}
