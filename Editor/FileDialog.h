#pragma once

#include <Core/Platform/FileInfo.h>
#include <Editor/MessageBox.h>
#include <string>
#include <vector>

namespace Columbus
{

	class EditorFileDialog
	{
	private:
		bool Opened = false;
		bool Hidden = false;
		bool Multiple = false;
		
		std::string Path;
		std::vector<FileInfo> SelectedFiles;

		MessageBox* Message = nullptr;
	public:
		EditorFileDialog() : Path("./") {}
		EditorFileDialog(const std::string& Path) : Path(Path) {}

		void Open() { Opened = true; }
		void Close() { Opened = false; }
		void ShowHidden(bool Show) { Hidden = Show; }
		void MultipleSelect(bool Select) { Multiple = Select; }
		void SetMessageBox(MessageBox* Box) { Message = Box; }
		bool Draw(const std::string& Name);
		std::vector<FileInfo> GetSelected() const { return SelectedFiles; }
	};

}


