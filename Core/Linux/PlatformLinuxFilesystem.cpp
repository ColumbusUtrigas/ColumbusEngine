#include <Core/Linux/PlatformLinuxFilesystem.h>
#include <cstdio>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace Columbus
{

	std::string FilesystemLinux::GetCurrent()
	{
		char dir[4096];
		if (getcwd(dir, 4096) == NULL) return "";
		return dir;
	}

	bool FilesystemLinux::CreateFile(std::string Path)
	{
		/*FILE* F = fopen(Path.c_str(), "w");
		if (F == nullptr) return false;
		fclose(F);
		return true;*/
		return open(Path.c_str(), O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, S_IRWXU) >= 0;
	}

	bool FilesystemLinux::CreateDirectory(std::string Path)
	{
		return mkdir(Path.c_str(), 0777) == 0;
	}

	bool FilesystemLinux::Rename(std::string Old, std::string New)
	{
		return std::rename(Old.c_str(), New.c_str()) == 0;
	}

	bool FilesystemLinux::RemoveFile(std::string Path)
	{
		return remove(Path.c_str()) == 0;
	}

	bool FilesystemLinux::RemoveDirectory(std::string Path)
	{
		return rmdir(Path.c_str()) == 0;
	}

}









