#include <Core/Windows/PlatformWindowsFilesystem.h>
#include <windows.h>
#include <cstring>

namespace Columbus
{

	const char* FilesystemWindows::GetCurrent()
	{
		char Buffer[MAX_PATH];
		DWORD Size = GetModuleFileName(NULL, Buffer, MAX_PATH);
		unsigned int Length = Size - strlen(strrchr(Buffer, '\\') + 1);
		char* Result = (char*)malloc(Length);
		Result[0] = '\0';
		return strncpy(Result, Buffer, Length);
	}

	bool FilesystemWindows::DirCreate(const char* Path)
	{
		return CreateDirectory(Path, NULL);
	}

	bool FilesystemWindows::DirRemove(const char* Path)
	{
		return RemoveDirectory(Path);
	}

}



