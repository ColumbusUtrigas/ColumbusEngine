#pragma once

#include <string>

namespace Columbus
{

	struct ClipboardWindows
	{
		static void SetClipboard(std::string Text);
		static void GetClipboard(std::string& Text);
		static bool HasClipboard();
	};

}














