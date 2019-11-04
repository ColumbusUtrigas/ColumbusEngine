#pragma once

namespace Columbus
{

	class EditorPanel
	{
	protected:
		friend class Editor;
		bool Opened = true;
	public:
		virtual ~EditorPanel() {}
	};

}


