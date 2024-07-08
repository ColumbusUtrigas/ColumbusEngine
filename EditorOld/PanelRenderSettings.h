#pragma once

#include <Editor/Panel.h>
#include <Graphics/Render.h>

namespace Columbus::Editor
{

	class PanelRenderSettings : public Panel
	{
	private:
		Renderer* Render = nullptr;

		void DrawInternal() final override;
	public:
		PanelRenderSettings();

		void SetRenderer(Renderer* Rend)
		{
			Render = Rend;
		}

		virtual ~PanelRenderSettings() final override;
	};

}
