#pragma once

#include <Core/Platform/FileInfo.h>
#include <string>
#include <vector>

namespace Columbus
{

	class EditorFileDialog
	{
	private:
		bool Opened = false;
		bool Hidden = false;
		
		std::string Path;
		FileInfo SelectedFile;
	public:
		EditorFileDialog() : Path("./") {}
		EditorFileDialog(const std::string& Path) : Path(Path) {}

		void Open() { Opened = true; }
		void Close() { Opened = false; }
		void ShowHidden(bool Show) { Hidden = Show; };
		bool Draw(const std::string& Name);
		FileInfo GetSelected() const { return SelectedFile; };
	};

}


