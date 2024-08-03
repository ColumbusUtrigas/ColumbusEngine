#pragma once

#include <Editor/Panel.h>
#include <cstddef>

namespace Columbus::Editor
{

	class PanelConsole : public Panel
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

		void DrawInternal() final override;
	public:
		PanelConsole();
		virtual ~PanelConsole() final override;
	};

}
