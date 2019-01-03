#pragma once

namespace Columbus
{
	
	struct Filesystem
	{
		static const char* GetCurrent();
		static bool FileCreate(const char* Path);
		static bool DirCreate(const char* Path);
		static bool Rename(const char* Old, const char* New);
		static bool FileRemove(const char* Path);
		static bool DirRemove(const char* Path);
	};

}









