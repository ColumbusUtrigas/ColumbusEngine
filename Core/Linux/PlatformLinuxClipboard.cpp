#include <Core/Linux/PlatformLinuxClipboard.h>
#include <SDL2/SDL.h>

namespace Columbus
{

	void ClipboardLinux::SetClipboard(const char* Text)
	{
		SDL_SetClipboardText(Text);
	}

	const char* ClipboardLinux::GetClipboard()
	{
		return SDL_GetClipboardText();
	}

	bool ClipboardLinux::HasClipboard()
	{
		return SDL_HasClipboardText();
	}

}


