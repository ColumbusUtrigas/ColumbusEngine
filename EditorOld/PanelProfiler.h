#pragma once

#include <Editor/Panel.h>

namespace Columbus::Editor
{

	class PanelProfiler : public Panel
	{
	private:
		static constexpr int GraphCount = 600;

		float RedrawTime = 0.0f;
		float T = 0.0f;
		float RedrawTimeCurve[GraphCount] = { 0.0f };
		int Index = 0;
		int ZeroIndex = 0;

		void DrawInternal() final override;
	public:
		PanelProfiler();

		void SetRedrawTime(float Time);
		
		virtual ~PanelProfiler() final override;
	};

}
