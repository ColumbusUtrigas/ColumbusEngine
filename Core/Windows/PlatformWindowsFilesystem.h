#pragma once

#include <Core/Platform/FileInfo.h>
#include <string>
#include <vector>

namespace Columbus
{

	struct FilesystemWindows
	{
		static std::string GetCurrent();
		static std::string AbsolutePath(const std::string& Path);
		static bool DirCreate(const char* Path);
		static bool DirRemove(const char* Path);
		static std::vector<FileInfo> Read(const std::string& Path);
	};

}


