#include <Core/Platform/Platform.h>
#include <Core/Platform/PlatformFilesystem.h>

#if defined(COLUMBUS_PLATFORM_WINDOWS)
	#include <Core/Windows/PlatformWindowsFilesystem.h>
#elif defined(COLUMBUS_PLATFORM_LINUX)
	#include <Core/Linux/PlatformLinuxFilesystem.h>
#elif defined(COLUMBUS_PLATFORM_APPLE)

#endif

namespace Columbus
{

	std::string Filesystem::GetCurrent()
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return FilesystemWindows::GetCurrent();
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return FilesystemLinux::GetCurrent();
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return "";
	}

	bool Filesystem::CreateFile(std::string Path)
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return FilesystemWindows::CreateFile(Path);
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return FilesystemLinux::CreateFile(Path);
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return false;
	}

	bool Filesystem::CreateDirectory(std::string Path)
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return FilesystemWindows::CreateDirectory(Path);
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return FilesystemLinux::CreateDirectory(Path);
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return false;
	}

	bool Filesystem::Rename(std::string Old, std::string New)
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return FilesystemWindows::Rename(Old, New);
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return FilesystemLinux::Rename(Old, New);
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return false;
	}

	bool Filesystem::RemoveFile(std::string Path)
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return FilesystemWindows::RemoveFile(Path);
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return FilesystemLinux::RemoveFile(Path);
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return false;
	}

	bool Filesystem::RemoveDirectory(std::string Path)
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return FilesystemWindows::RemoveDirectory(Path);
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return FilesystemLinux::RemoveDirectory(Path);
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return false;
	}

}












