#pragma once

#include <Editor/Panel.h>
#include <Scene/GameObject.h>
#include <Scene/Scene.h>

namespace Columbus::Editor
{

	class PanelInspector : public Panel
	{
	private:
		Scene* _Scene = nullptr;
		GameObject* Inspectable = nullptr;

		void DrawInternal() final override;

		void DrawAddComponent(Scene& Scn);
		void DrawTransformEditor();
		void DrawMaterialEditor(Scene& Scn);
		void DrawComponentsEditor(Scene& Scn);
	public:
		PanelInspector();

		void SetScene(Scene* scene)
		{
			_Scene = scene;
		}

		void SetInspectableObject(GameObject* Object)
		{
			Inspectable = Object;
		}

		~PanelInspector() final override;
	};

}
