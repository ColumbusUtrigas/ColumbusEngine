#pragma once

#include <Editor/Panel.h>
#include <Graphics/Texture.h>
#include <Math/Vector2.h>

namespace Columbus
{

	class EditorPanelScene : public EditorPanel
	{
	private:
		Texture* FramebufferTexture = nullptr;
		iVector2 SizeOfRenderWindow;
		bool Stats = false;
		bool Selected = false;
		bool Hover = false;
	public:
		EditorPanelScene() : EditorPanel("Scene") {}

		void SetFramebufferTexture(Texture* Tex)
		{
			FramebufferTexture = Tex;
		}

		iVector2 GetSize() const
		{
			return SizeOfRenderWindow;
		}

		bool IsSelected() const { return Selected; }
		bool IsHover() const { return Hover; }

		virtual void Draw() final override;

		virtual ~EditorPanelScene() {}
	};

}


