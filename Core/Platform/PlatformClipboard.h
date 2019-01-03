#pragma once

namespace Columbus
{

	struct Clipboard
	{
		static void SetClipboard(const char* Text);
		static void GetClipboard(char*& Text);
		static bool HasClipboard();
	};

}










