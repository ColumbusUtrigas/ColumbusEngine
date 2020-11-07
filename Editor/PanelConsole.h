#pragma once

#include <Editor/Panel.h>
#include <cstddef>

namespace Columbus::Editor
{

	class PanelConsole : public EditorPanel
	{
	private:
		bool _EnableInit = false;
		bool _EnableSuccess = true;
		bool _EnableMessage = true;
		bool _EnableWarning = true;
		bool _EnableError = true;
		bool _EnableFatal = false;

		bool _lastWasEnd = false;
		float _lastWheel = 0.0f;
		size_t _lastCount = 0;
	public:
		PanelConsole() {}

		void Draw();
		
		virtual ~PanelConsole() final override {}
	};

}


