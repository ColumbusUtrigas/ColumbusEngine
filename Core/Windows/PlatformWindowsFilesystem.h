#pragma once

namespace Columbus
{

	struct FilesystemWindows
	{
		static const char* GetCurrent();
		static bool DirCreate(const char* Path);
		static bool DirRemove(const char* Path);
	};

}


