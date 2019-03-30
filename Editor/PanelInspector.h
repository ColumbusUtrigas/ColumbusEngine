#pragma once

#include <Editor/Panel.h>
#include <Scene/GameObject.h>

namespace Columbus
{

	class EditorPanelInspector : public EditorPanel
	{
	private:
		GameObject* Inspectable = nullptr;
	public:
		EditorPanelInspector() : EditorPanel("Inspector") {}
		void SetInspectableObject(GameObject* Object)
		{
			Inspectable = Object;
		}

		virtual void Draw() final override;
		virtual ~EditorPanelInspector() final override;
	};

}


