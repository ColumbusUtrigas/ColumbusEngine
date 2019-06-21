#pragma once

#include <string>

namespace Columbus
{

	class EditorPanel
	{
	protected:
		friend class Editor;

		std::string Name;
		bool Opened = true;
	public:
		EditorPanel(const std::string& Name = "Panel") : Name(Name) {}

		virtual void Draw();

		virtual ~EditorPanel() {}
	};

}


