#include <Core/Platform/Platform.h>
#include <Core/Platform/PlatformClipboard.h>

#if defined(COLUMBUS_PLATFORM_WINDOWS)
	#include <Core/Windows/PlatformWindowsClipboard.h>
#elif defined(COLUMBUS_PLATFORM_LINUX)
	#include <Core/Linux/PlatformLinuxClipboard.h>
#elif defined(COLUMBUS_PLATFORM_APPLE)

#endif

namespace Columbus
{

	void Clipboard::SetClipboard(const char* Text)
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			ClipboardWindows::SetClipboard(Text);
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			ClipboardLinux::SetClipboard(Text);
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif
	}

	const char* Clipboard::GetClipboard()
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return ClipboardWindows::GetClipboard();
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return ClipboardLinux::GetClipboard();
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return "";
	}

	bool Clipboard::HasClipboard()
	{
		#if defined(COLUMBUS_PLATFORM_WINDOWS)
			return ClipboardWindows::HasClipboard();
		#elif defined(COLUMBUS_PLATFORM_LINUX)
			return ClipboardLinux::HasClipboard();
		#elif defined(COLUMBUS_PLATFORM_APPLE)

		#endif

		return false;
	}

}














