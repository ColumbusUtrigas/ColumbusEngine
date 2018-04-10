#include <GUI/Button.h>
#include <iostream>

namespace Columbus
{

	namespace GUI
	{

		void Button::update()
		{
			mState = 0;

			if (mIO.mouse.enabled == false)
				return;

			Vector2 m = mIO.mouse.coords;

			if (m.X > mPos.X && m.X < (mPos.X + mSize.Y))
			{
				if (m.Y > mPos.Y && m.Y < (mPos.Y + mSize.Y))
				{
					mState = 1;
				}
			}
		}

		void Button::draw()
		{
			Vector2 pos = mPos / mIO.screen.size;
			Vector2 size = mSize / mIO.screen.size * 2;

			//C_DisableDepthMaskOpenGL();

			glColor4f(1, 0, 0, 1);

			glBegin(GL_QUADS);
				glVertex2f(pos.X, pos.Y);
				glVertex2f(pos.X + size.X, pos.Y);
				glVertex2f(pos.X + size.X, pos.Y + size.Y);
				glVertex2f(pos.X, pos.Y + size.Y);
			glEnd();

			//C_EnableDepthMaskOpenGL();
		}

	}

}
