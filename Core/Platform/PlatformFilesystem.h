#pragma once

#include <string>

namespace Columbus
{
	
	struct Filesystem
	{
		static std::string GetCurrent();
		static bool CreateFile(std::string Path);
		static bool CreateDirectory(std::string Path);
		static bool Rename(std::string Old, std::string New);
		static bool RemoveFile(std::string Path);
		static bool RemoveDirectory(std::string Path);
	};

}









