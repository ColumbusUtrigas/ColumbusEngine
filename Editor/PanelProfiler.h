#pragma once

#include <Editor/Panel.h>

namespace Columbus
{

	class EditorPanelProfiler : public EditorPanel
	{
	private:
		float RedrawTime = 0.0f;
	public:
		EditorPanelProfiler() : EditorPanel("Profiler") {}

		void SetRedrawTime(float Time)
		{
			RedrawTime = Time;
		}

		virtual void Draw() final override;
		virtual ~EditorPanelProfiler() final override;
	};

}


