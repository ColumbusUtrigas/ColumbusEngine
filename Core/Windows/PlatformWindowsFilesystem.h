#pragma once

#include <Core/Platform/FileInfo.h>
#include <vector>

namespace Columbus
{

	struct FilesystemWindows
	{
		static String GetCurrent();
		static String AbsolutePath(const String& Path);
		static bool DirCreate(const char* Path);
		static bool DirRemove(const char* Path);
		static std::vector<FileInfo> Read(const String& Path);
	};

}


