#pragma once

#include <Editor/Panel.h>
#include <Graphics/Texture.h>
#include <Graphics/Render.h>
#include <Math/Vector2.h>

namespace Columbus::Editor
{

	class PanelScene : public Panel
	{
	private:
		Texture* FramebufferTexture = nullptr;
		iVector2 Position;
		iVector2 SizeOfRenderWindow;
		bool Selected = false;
		bool Hover = false;

		Scene* _Scene;
		Renderer* _Renderer;
		Gizmo _Gizmo;
		GameObject* _PickedObject = nullptr;

		void DrawInternal() final override;
	public:
		PanelScene();

		void SetPickedObject(GameObject* Obj)
		{
			_PickedObject = Obj;
		}

		void SetFramebufferTexture(Texture* Tex)
		{
			FramebufferTexture = Tex;
		}

		void SetScene(Scene* scene)
		{
			_Scene = scene;
		}

		void SetRenderer(Renderer* renderer)
		{
			_Renderer = renderer;
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

		virtual ~PanelScene() final override {}
	};

}
