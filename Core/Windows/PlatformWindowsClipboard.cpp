#include <Core/Windows/PlatformWindowsClipboard.h>
#include <Core/Memory.h>
#include <windows.h>

namespace Columbus
{

	void ClipboardWindows::SetClipboard(const char* Text)
	{
		const uint32 len = strlen(Text) + 1;
		HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
		Memory::Memcpy(GlobalLock(hMem), Text, len);
		GlobalUnlock(hMem);
		OpenClipboard(nullptr);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}

	const char* ClipboardWindows::GetClipboard()
	{
		OpenClipboard(nullptr);
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData == nullptr)
		{
			GlobalUnlock(hData);
			CloseClipboard();
			return "";
		}
		char* Text = (char*)(GlobalLock(hData));
		GlobalUnlock(hData);
		CloseClipboard();
		return Text;
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















