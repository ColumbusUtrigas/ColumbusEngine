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

	bool Filesystem::FileCreate(const char* Path)
	{
		FILE* File = fopen(Path, "w");
		bool Result = File != nullptr;
		if (File != nullptr) fclose(File);
		return Result;
	}

	bool Filesystem::DirCreate(const char* Path)
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return FilesystemWindows::DirCreate(Path);
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return FilesystemLinux::DirCreate(Path);
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return false;
	}

	bool Filesystem::Rename(const char* Old, const char* New)
	{
		return rename(Old, New) == 0;
	}

	bool Filesystem::FileRemove(const char* Path)
	{
		return remove(Path) == 0;
	}

	bool Filesystem::DirRemove(const char* Path)
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return FilesystemWindows::DirRemove(Path);
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return FilesystemLinux::DirRemove(Path);
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return false;
	}

}












