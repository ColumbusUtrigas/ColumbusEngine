#pragma once

#include <string>
#include <vector>
#include <Core/Platform/FileInfo.h>
#include <Core/String.h>

namespace Columbus
{
	
	struct Filesystem
	{
		static String GetCurrent();
		static String AbsolutePath(const String& Path);
		static String RelativePath(const String& Absolute, const String& RelativeTo);
		static bool FileCreate(const char* Path);
		static bool DirCreate(const char* Path);
		static bool Rename(const char* Old, const char* New);
		static bool FileRemove(const char* Path);
		static bool DirRemove(const char* Path);
		static std::vector<FileInfo> Read(const String& Path);
		static std::vector<String> Split(const String& Path);
	};

}


