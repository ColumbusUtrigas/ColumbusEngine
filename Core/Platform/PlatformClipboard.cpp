#include <Core/Platform/Platform.h>
#include <Core/Platform/PlatformClipboard.h>

#if defined(PLATFORM_WINDOWS)
	#include <Core/Windows/PlatformWindowsClipboard.h>
#elif defined(PLATFORM_LINUX)
	#include <Core/Linux/PlatformLinuxClipboard.h>
#elif defined(PLATFORM_APPLE)

#endif

namespace Columbus
{

	void Clipboard::SetClipboard(const char* Text)
	{
		#if defined(PLATFORM_WINDOWS)
			ClipboardWindows::SetClipboard(Text);
		#elif defined(PLATFORM_LINUX)
			ClipboardLinux::SetClipboard(Text);
		#elif defined(PLATFORM_APPLE)

		#endif
	}

	const char* Clipboard::GetClipboard()
	{
		#if defined(PLATFORM_WINDOWS)
			return ClipboardWindows::GetClipboard();
		#elif defined(PLATFORM_LINUX)
			return ClipboardLinux::GetClipboard();
		#elif defined(PLATFORM_APPLE)

		#endif

		return "";
	}

	bool Clipboard::HasClipboard()
	{
		#if defined(PLATFORM_WINDOWS)
			return ClipboardWindows::HasClipboard();
		#elif defined(PLATFORM_LINUX)
			return ClipboardLinux::HasClipboard();
		#elif defined(PLATFORM_APPLE)

		#endif

		return false;
	}

}


