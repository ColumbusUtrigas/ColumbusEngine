#pragma once

namespace Columbus
{

	struct FilesystemLinux
	{
		static const char* GetCurrent();
		static bool DirCreate(const char* Path);
		static bool DirRemove(const char* Path);
	};

}







