#pragma once

#include <GUI/Widget.h>
#include <Graphics/Texture.h>
#include <GL/glew.h>

namespace Columbus
{

	class Button : public Widget
	{
	public:
		Texture* MainTexture = nullptr;
	public:
		Button() {}
		Button(const Vector2& InPosition, const Vector2& InSize) : Widget(InPosition, InSize) {}

		virtual void Prepare() override
		{
			
		}

		virtual void Render() override
		{
			Vector2 ULCorner = Position - Size * 0.5f;
			Vector2 BRCorner = Position + Size * 0.5f;

			glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);
				glVertex2f(ULCorner.X, ULCorner.Y);
				glTexCoord2f(1.0, 0.0);
				glVertex2f(BRCorner.X, ULCorner.Y);
				glTexCoord2f(1.0, 1.0);
				glVertex2f(BRCorner.X, BRCorner.Y);
				glTexCoord2f(0.0, 1.0);
				glVertex2f(ULCorner.X, BRCorner.Y);
			glEnd();
		}

		~Button() {}
	};

}
