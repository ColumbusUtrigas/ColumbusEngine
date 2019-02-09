#pragma once

namespace Columbus
{

	struct ClipboardLinux
	{
		static void SetClipboard(const char* Text);
		static const char* GetClipboard();
		static bool HasClipboard();
	};

}










