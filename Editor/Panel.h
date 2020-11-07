#pragma once

namespace Columbus::Editor
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


