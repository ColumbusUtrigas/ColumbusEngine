#pragma once

namespace Columbus
{

	struct Clipboard
	{
		static void SetClipboard(const char* Text);
		static const char* GetClipboard();
		static bool HasClipboard();
	};

}










