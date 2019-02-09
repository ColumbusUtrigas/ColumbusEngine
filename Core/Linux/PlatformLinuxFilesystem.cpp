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
		char dir[4096];
		if (getcwd(dir, 4096) == NULL) return "";
		return dir;
	}

	bool FilesystemLinux::DirCreate(const char* Path)
	{
		return mkdir(Path, 0777) == 0;
	}

	bool FilesystemLinux::DirRemove(const char* Path)
	{
		return rmdir(Path) == 0;
	}

}









