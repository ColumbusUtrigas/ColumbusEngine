#pragma once

namespace Columbus
{

	struct ClipboardWindows
	{
		static void SetClipboard(const char* Text);
		static const char* GetClipboard();
		static bool HasClipboard();
	};

}














