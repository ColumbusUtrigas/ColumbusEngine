#include <Core/Linux/PlatformLinuxFilesystem.h>
#include <cstdio>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace Columbus
{

	const char* FilesystemLinux::GetCurrent()
	{
		char* dir = new char[4096];
		if (getcwd(dir, 4096) == NULL) return "";
		return dir;
	}

	const char* FilesystemLinux::AbsolutePath(const char* Path)
	{
		return realpath(Path, NULL);
	}

	bool FilesystemLinux::DirCreate(const char* Path)
	{
		return mkdir(Path, 0777) == 0;
	}

	bool FilesystemLinux::DirRemove(const char* Path)
	{
		return rmdir(Path) == 0;
	}

	std::vector<FileInfo> FilesystemLinux::Read(const std::string& Path)
	{
		std::vector<FileInfo> Result;

		DIR* dir = opendir(Path.c_str());
		if (dir == NULL) return Result;

		dirent* dptr = NULL;

		while ((dptr = readdir(dir)) != NULL)
		{
			FileInfo Info;

			std::string Name = dptr->d_name;
			unsigned char Type = dptr->d_type;

			auto Pos = Name.rfind('.');

			Info.Name = Name;
			Info.Ext = (Pos != std::string::npos && Pos != 0) ? Name.substr(Pos + 1) : "";
			Info.Path = Path[Path.length() - 1] == '/' ? Path + Name : Path + '/' + Name;

			switch (Type)
			{
			case DT_DIR: Info.Type = 'd'; break;
			case DT_LNK: Info.Type = 'l'; break;
			case DT_REG: Info.Type = 'f'; break;
			}
			
			Result.push_back(Info);
		}

		closedir(dir);

		return Result;
	}

}


