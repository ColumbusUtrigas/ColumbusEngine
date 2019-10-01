#pragma once

#include <Editor/Panel.h>

namespace Columbus
{

	class EditorPanelConsole : public EditorPanel
	{
	private:
		bool _EnableInit = false;
		bool _EnableSuccess = true;
		bool _EnableMessage = true;
		bool _EnableWarning = true;
		bool _EnableError = true;
		bool _EnableFatal = false;
	public:
		EditorPanelConsole() {}

		void Draw();
		
		virtual ~EditorPanelConsole() final override {}
	};

}


