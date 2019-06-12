#pragma once

#include <Core/Platform/FileInfo.h>
#include <string>
#include <vector>

namespace Columbus
{

	struct FilesystemLinux
	{
		static const char* GetCurrent();
		static const char* AbsolutePath(const char* Path);
		static bool DirCreate(const char* Path);
		static bool DirRemove(const char* Path);
		static std::vector<FileInfo> Read(const std::string& Path);
	};

}


