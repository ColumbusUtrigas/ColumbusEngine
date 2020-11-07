#pragma once

#include <Editor/Panel.h>
#include <Graphics/Texture.h>
#include <Graphics/Render.h>
#include <Math/Vector2.h>

namespace Columbus::Editor
{

	class PanelScene : public EditorPanel
	{
	private:
		Texture* FramebufferTexture = nullptr;
		iVector2 Position;
		iVector2 SizeOfRenderWindow;
		bool Selected = false;
		bool Hover = false;

		Gizmo _Gizmo;
		GameObject* _PickedObject = nullptr;
	public:
		PanelScene() {}

		void SetPickedObject(GameObject* Obj)
		{
			_PickedObject = Obj;
		}

		void SetFramebufferTexture(Texture* Tex)
		{
			FramebufferTexture = Tex;
		}

		iVector2 GetPosition() const
		{
			return Position;
		}

		iVector2 GetSize() const
		{
			return SizeOfRenderWindow;
		}

		bool IsSelected() const { return Selected; }
		bool IsHover() const { return Hover; }

		void Draw(Scene& Scene, Renderer& Render);

		virtual ~PanelScene() final override {}
	};

}


