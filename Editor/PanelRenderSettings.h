#pragma once

#include <Editor/Panel.h>
#include <Graphics/Render.h>

namespace Columbus::Editor
{

	class PanelRenderSettings : public EditorPanel
	{
	private:
		Renderer* Render = nullptr;
	public:
		PanelRenderSettings() {}

		void SetRenderer(Renderer* Rend)
		{
			Render = Rend;
		}

		void Draw();

		virtual ~PanelRenderSettings() final override;
	};

}


