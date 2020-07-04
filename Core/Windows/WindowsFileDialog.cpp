#include <Core/FileDialog.h>
#include <windows.h>
#include <stdio.h>

namespace Columbus
{

	void EngineOpenFileDialog()
	{
		OPENFILENAME ofn;       // common dialog box structure
		TCHAR szFile[260] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = TEXT("All\0*.*\0Text\0*.TXT\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn) == TRUE)
		{
			// use ofn.lpstrFile
		}
	}

	void EngineShowInFileExplorer(const char* path, ShowInExplorerType type)
	{
		char str[1024] = { 0 };
		strcat(str, path);
		int len = strlen(str);
		for (int i = 0; i < len; i++)
			if (str[i] == '/') str[i] = '\\';

		char* typestr = "/select";
		switch (type)
		{
		case ShowInExplorerType::Root: typestr = "/root";  break;
		case ShowInExplorerType::Select: typestr = "/select";  break;
		}

		char buf[1024] = { 0 };
		sprintf(buf, "explorer.exe %s,\"%s\"", typestr, str);
		system(buf);
	}

}
