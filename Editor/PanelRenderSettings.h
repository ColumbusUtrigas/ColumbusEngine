#pragma once

#include <Editor/Panel.h>
#include <Graphics/Render.h>

namespace Columbus
{

	class EditorPanelRenderSettings : public EditorPanel
	{
	private:
		Renderer* Render = nullptr;
	public:
		EditorPanelRenderSettings() : EditorPanel("Render Settings") {}

		void SetRenderer(Renderer* Rend)
		{
			Render = Rend;
		}

		virtual void Draw() final override;

		virtual ~EditorPanelRenderSettings() final override;
	};

}


