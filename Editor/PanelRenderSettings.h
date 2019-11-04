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
		EditorPanelRenderSettings() {}

		void SetRenderer(Renderer* Rend)
		{
			Render = Rend;
		}

		void Draw();

		virtual ~EditorPanelRenderSettings() final override;
	};

}


