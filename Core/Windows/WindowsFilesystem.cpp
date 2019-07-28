#include <Core/Filesystem.h>
#include <windows.h>
#include <string.h>

namespace Columbus
{

	String Filesystem::GetCurrent()
	{
		char Buffer[MAX_PATH];
		DWORD Size = GetModuleFileName(NULL, Buffer, MAX_PATH);
		unsigned int Length = Size - strlen(strrchr(Buffer, '\\') + 1);
		char* Result = (char*)malloc(Length);
		Result[0] = '\0';
		strncpy(Result, Buffer, Length);
		String Ret = Result;
		free(Result);
		return Ret;
	}

	String Filesystem::AbsolutePath(const String& Path)
	{
		char Result[MAX_PATH] = { '\0' };
		memset(Result, 0, MAX_PATH);
		GetFullPathName(Path.c_str(), MAX_PATH, Result, NULL);

		for (int i = 0; i < MAX_PATH; i++)
			if (Result[i] == '\\')
				Result[i] = '/';

		return Result;
	}

	std::vector<FileInfo> Filesystem::Read(const String& Path)
	{
		std::vector<FileInfo> Result;

		WIN32_FIND_DATA FFD;
		HANDLE hFind = FindFirstFile((Path + "*.*").c_str(), &FFD);

		if (hFind == INVALID_HANDLE_VALUE) printf("Invalid\n");

		do
		{
			FileInfo Info;

			String Name = FFD.cFileName;
			auto Pos = Name.rfind('.');

			Info.Name = Name;
			Info.Ext = (Pos != std::string::npos && Pos != 0) ? Name.substr(Pos + 1) : "";
			Info.Path = Path[Path.length() - 1] == '/' ? Path + Name : Path + '/' + Name;

			if (FFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				Info.Type = 'd';
			else
				Info.Type = 'f';

			Result.push_back(Info);
		} while (FindNextFile(hFind, &FFD) != 0);

		FindClose(hFind);

		return Result;
	}

}


