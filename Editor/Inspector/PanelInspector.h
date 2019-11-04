#pragma once

#include <Editor/Panel.h>
#include <Scene/GameObject.h>
#include <Scene/Scene.h>

namespace Columbus
{

	class EditorPanelInspector
	{
	private:
		friend class Editor;

		bool Opened = true;
		GameObject* Inspectable = nullptr;

		void DrawAddComponent(Scene& Scn);
		void DrawTransformEditor();
		void DrawMaterialEditor(Scene& Scn);
		void DrawComponentsEditor(Scene& Scn);
		void DrawComponentAudioEditor(Scene& Scn);
		void DrawComponentLightEditor(Scene& Scn);
		void DrawComponentMeshRendererEditor(Scene& Scn);
		void DrawComponentParticleSystemEditor(Scene& Scn);
		void DrawComponentRigidbodyEditor(Scene& Scn);
	public:
		void SetInspectableObject(GameObject* Object)
		{
			Inspectable = Object;
		}

		void Draw(Scene& Scn);
		~EditorPanelInspector();
	};

}


