#pragma once

namespace Columbus
{
	
	struct Filesystem
	{
		static const char* GetCurrent();
		static bool CreateFile(const char* Path);
		static bool CreateDirectory(const char* Path);
		static bool Rename(const char* Old, const char* New);
		static bool RemoveFile(const char* Path);
		static bool RemoveDirectory(const char* Path);
	};

}









