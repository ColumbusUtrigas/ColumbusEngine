#pragma once

#include <Editor/MessageBox.h>
#include <Core/FileInfo.h>
#include <Core/String.h>
#include <vector>
#include <string>

namespace Columbus
{

	class EditorFileDialog
	{
	public:
		enum Type
		{
			Type_Open,
			Type_Save
		};
	private:
		bool Opened = false;
		bool CloseFlag = false;
		bool Hidden = false;
		bool Multiple = false;
		
		Type _Type;
		String Path;
		FileInfo SaveFile;
		std::vector<std::string> _Filter;
		std::vector<FileInfo> SelectedFiles;

		MessageBox* Message = nullptr;
		void _Open();
	public:
		EditorFileDialog() : Path("./") {}
		EditorFileDialog(const String& Path, const std::vector<std::string>& Filter = {})
			: Path(Path), _Filter(Filter) {}

		void Open(Type type = Type_Open)
		{
			Opened = true;
			_Type = type;
			SelectedFiles.clear();
			SaveFile = FileInfo();
			_Open();
		}
		void Close() { CloseFlag = true; }
		void ShowHidden(bool Show) { Hidden = Show; }
		void MultipleSelect(bool Select) { Multiple = Select; }
		void SetMessageBox(MessageBox* Box) { Message = Box; }
		bool Draw(const String& Name);
		std::vector<FileInfo> GetSelected() const { return SelectedFiles; }
		Type GetType() const { return _Type; }
	};

}


