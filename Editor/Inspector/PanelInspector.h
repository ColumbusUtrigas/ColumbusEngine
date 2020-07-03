#pragma once

#include <Editor/Panel.h>
#include <Scene/GameObject.h>
#include <Scene/Scene.h>

namespace Columbus
{

	class EditorPanelInspector : public EditorPanel
	{
	private:
		GameObject* Inspectable = nullptr;

		void DrawAddComponent(Scene& Scn);
		void DrawTransformEditor();
		void DrawMaterialEditor(Scene& Scn);
		void DrawComponentsEditor(Scene& Scn);
	public:
		void SetInspectableObject(GameObject* Object)
		{
			Inspectable = Object;
		}

		void Draw(Scene& Scn);
		~EditorPanelInspector();
	};

}


