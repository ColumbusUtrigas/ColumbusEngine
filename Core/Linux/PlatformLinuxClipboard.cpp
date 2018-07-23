#include <Core/Linux/PlatformLinuxClipboard.h>
#include <SDL2/SDL.h>

namespace Columbus
{

	void ClipboardLinux::SetClipboard(std::string Text)
	{
		SDL_SetClipboardText(Text.c_str());
	}

	void ClipboardLinux::GetClipboard(std::string& Text)
	{
		Text = SDL_GetClipboardText();
	}

	bool ClipboardLinux::HasClipboard()
	{
		return SDL_HasClipboardText();
	}

}













