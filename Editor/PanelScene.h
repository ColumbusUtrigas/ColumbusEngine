#pragma once

#include <Editor/Panel.h>
#include <Graphics/Texture.h>
#include <Graphics/Render.h>
#include <Math/Vector2.h>

namespace Columbus
{

	class EditorPanelScene : public EditorPanel
	{
	private:
		Texture* FramebufferTexture = nullptr;
		iVector2 Position;
		iVector2 SizeOfRenderWindow;
		bool Stats = false;
		bool Selected = false;
		bool Hover = false;
	public:
		EditorPanelScene() {}

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

		void Draw(Renderer& Render);

		virtual ~EditorPanelScene() final override {}
	};

}


