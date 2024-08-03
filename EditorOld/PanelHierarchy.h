#pragma once

#include <Editor/Panel.h>
#include <Scene/Scene.h>

namespace Columbus::Editor
{

	class PanelHierarchy : public Panel
	{
	private:
		Scene* scene = nullptr;
		GameObject* object = nullptr;
		GameObject* buffer = nullptr;
		GameObject* buffer2 = nullptr;

		void DrawInternal() final override;
	public:
		PanelHierarchy();

		void SetScene(Scene* Scn)
		{
			scene = Scn;
		}

		GameObject* GetCurrentObject() const
		{
			return object;
		}

		virtual ~PanelHierarchy() final override;
	};

}
