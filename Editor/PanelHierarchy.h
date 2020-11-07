#pragma once

#include <Editor/Panel.h>
#include <Scene/Scene.h>

namespace Columbus::Editor
{

	class PanelHierarchy : public EditorPanel
	{
	private:
		Scene* scene = nullptr;
		GameObject* object = nullptr;
		GameObject* buffer = nullptr;
		GameObject* buffer2 = nullptr;
	public:
		PanelHierarchy() {}

		void SetScene(Scene* Scn)
		{
			scene = Scn;
		}

		GameObject* GetCurrentObject() const
		{
			return object;
		}

		void Draw();

		virtual ~PanelHierarchy() final override {}
	};

}


