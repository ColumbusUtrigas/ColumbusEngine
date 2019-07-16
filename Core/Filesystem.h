#pragma once

#include <Core/FileInfo.h>
#include <Core/String.h>
#include <vector>

namespace Columbus
{

	struct Filesystem
	{
		static String GetCurrent();
		static String AbsolutePath(const String& Path);
		static String RelativePath(const String& Absolute, const String& RelativeTo);
		static std::vector<FileInfo> Read(const String& Path);
		static std::vector<String> Split(const String& Path);
	};

}


