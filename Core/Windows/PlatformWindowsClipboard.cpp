#include <Core/Windows/PlatformWindowsClipboard.h>
#include <Core/Memory.h>
#include <windows.h>

namespace Columbus
{

	void ClipboardWindows::SetClipboard(std::string Text)
	{
		const uint32 len = Text.size() + 1;
		HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
		Memory::Memcpy(GlobalLock(hMem), Text.c_str(), len);
		GlobalUnlock(hMem);
		OpenClipboard(nullptr);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}

	void ClipboardWindows::GetClipboard(std::string& Text)
	{
		OpenClipboard(nullptr);
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData == nullptr)
		{
			Text.clear();
			GlobalUnlock(hData);
			CloseClipboard();
			return;
		}
		char* pszText = (char*)(GlobalLock(hData));
		Text = pszText;
		GlobalUnlock(hData);
		CloseClipboard();
	}

	bool ClipboardWindows::HasClipboard()
	{
		OpenClipboard(nullptr);
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData == nullptr)
		{
			GlobalUnlock(hData);
			CloseClipboard();
			return false;
		}
		
		GlobalUnlock(hData);
		CloseClipboard();
		return true;
	}

}















