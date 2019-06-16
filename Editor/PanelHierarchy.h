#pragma once

#include <Editor/Panel.h>
#include <Scene/Scene.h>

namespace Columbus
{

	class EditorPanelHierarchy : public EditorPanel
	{
	private:
		Scene* scene = nullptr;
		GameObject* object = nullptr;
		GameObject* buffer = nullptr;
	public:
		EditorPanelHierarchy() : EditorPanel("Hierarchy") {}

		void SetScene(Scene* Scn)
		{
			scene = Scn;
		}

		GameObject* GetObject() const
		{
			return object;
		}

		virtual void Draw() final override;

		virtual ~EditorPanelHierarchy() final override {}
	};

}


