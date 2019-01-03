#include <Core/Platform/Platform.h>
#include <Core/Platform/PlatformFilesystem.h>
#include <cstdio>

#if defined(COLUMBUS_PLATFORM_WINDOWS)
	#include <Core/Windows/PlatformWindowsFilesystem.h>
#elif defined(COLUMBUS_PLATFORM_LINUX)
	#include <Core/Linux/PlatformLinuxFilesystem.h>
#elif defined(COLUMBUS_PLATFORM_APPLE)

#endif

namespace Columbus
{

	const char* Filesystem::GetCurrent()
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return FilesystemWindows::GetCurrent();
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return FilesystemLinux::GetCurrent();
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return "";
	}

	bool Filesystem::CreateFile(const char* Path)
	{
		FILE* File = fopen(Path, "w");
		bool Result = File != nullptr;
		if (File != nullptr) fclose(File);
		return Result;
	}

	bool Filesystem::CreateDirectory(const char* Path)
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return FilesystemWindows::DirCreate(Path);
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return FilesystemLinux::CreateDirectory(Path);
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return false;
	}

	bool Filesystem::Rename(const char* Old, const char* New)
	{
		return rename(Old, New) == 0;
	}

	bool Filesystem::RemoveFile(const char* Path)
	{
		return remove(Path) == 0;
	}

	bool Filesystem::RemoveDirectory(const char* Path)
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return FilesystemWindows::DirRemove(Path);
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return FilesystemLinux::RemoveDirectory(Path);
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return false;
	}

}












