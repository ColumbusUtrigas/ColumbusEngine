#include <Core/Windows/PlatformWindowsFilesystem.h>
#include <windows.h>
#include <cstring>

namespace Columbus
{

	std::string FilesystemWindows::GetCurrent()
	{
		char Buffer[MAX_PATH];
		DWORD Size = GetModuleFileName(NULL, Buffer, MAX_PATH);
		unsigned int Length = Size - strlen(strrchr(Buffer, '\\') + 1);
		char* Result = (char*)malloc(Length);
		Result[0] = '\0';
		strncpy(Result, Buffer, Length);
		std::string Ret = Result;
		free(Result);
		return Ret;
	}

	std::string FilesystemWindows::AbsolutePath(const std::string& Path)
	{
		char Result[MAX_PATH] = { '\0' };
		memset(Result, 0, MAX_PATH);
		GetFullPathName(Path.c_str(), MAX_PATH, Result, NULL);
		return Result;
	}

	bool FilesystemWindows::DirCreate(const char* Path)
	{
		return CreateDirectory(Path, NULL);
	}

	bool FilesystemWindows::DirRemove(const char* Path)
	{
		return RemoveDirectory(Path);
	}

	std::vector<FileInfo> FilesystemWindows::Read(const std::string& Path)
	{
		std::vector<FileInfo> Result;

		WIN32_FIND_DATA FFD;
		HANDLE hFind = FindFirstFile(Path.c_str(), &FFD);

		do
		{
			FileInfo Info;

			std::string Name = FFD.cFileName;
			auto Pos = Name.rfind('.');

			Info.Name = Name;
			Info.Ext = (Pos != std::string::npos && Pos != 0) ? Name.substr(Pos + 1) : "";
			Info.Path = Path[Path.length() - 1] == '/' ? Path + Name : Path + '/' + Name;

			if (FFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) Info.Type = 'd';
			if (FFD.dwFileAttributes & FILE_ATTRIBUTE_NORMAL)    Info.Type = 'f';

			Result.push_back(Info);
		} while (FindNextFile(hFind, &FFD) != 0);

		return Result;
	}

}


