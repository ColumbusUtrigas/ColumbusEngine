#pragma once

#include <string>

namespace Columbus
{

	struct Clipboard
	{
		static void SetClipboard(std::string Text);
		static void GetClipboard(std::string& Text);
		static bool HasClipboard();
	};

}










