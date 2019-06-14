#pragma once

#include <string>
#include <vector>
#include <Core/Platform/FileInfo.h>

namespace Columbus
{
	
	struct Filesystem
	{
		static const char* GetCurrent();
		static const char* AbsolutePath(const char* Path);
		static std::string RelativePath(const std::string& Absolute, const std::string& RelativeTo);
		static bool FileCreate(const char* Path);
		static bool DirCreate(const char* Path);
		static bool Rename(const char* Old, const char* New);
		static bool FileRemove(const char* Path);
		static bool DirRemove(const char* Path);
		static std::vector<FileInfo> Read(const std::string& Path);
		static std::vector<std::string> Split(const std::string& Path);
	};

}


