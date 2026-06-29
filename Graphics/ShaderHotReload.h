#pragma once

#include <ShaderBytecode/ShaderBytecode.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace Columbus
{
	class ShaderCache;

	struct ShaderHotReloadState
	{
		std::filesystem::file_time_type LastWriteTime;
		bool bHasLastWriteTime = false;
		uint64_t SeenReloadGeneration = 0;
	};

	struct ShaderHotReload
	{
		void Update(ShaderCache& Cache);
		bool ReloadShaderDataIfNeeded(
			const char* Path,
			CompiledShaderData& PackedBytecode,
			bool& bLoadedBytecode);

	private:
		uint64_t SeenReloadGeneration = 0;
		std::unordered_map<std::string, ShaderHotReloadState> ShaderStates;
	};
}
