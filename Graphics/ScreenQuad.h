#pragma once

#include <Core/Types.h>
#include <GL/glew.h>

namespace Columbus
{

	class ScreenQuad
	{
	public:
		ScreenQuad() {}

		void Render(const Vector2& Position = { 0 }, const Vector2& Size = { 1 })
		{
			glDisable(GL_CULL_FACE);
			glDepthMask(GL_FALSE);

			glBegin(GL_QUADS);
				glTexCoord2f(0, 1);
				glVertex2f(Position.X - Size.X, Position.Y + Size.Y);

				glTexCoord2f(1, 1);
				glVertex2f(Position.X + Size.X, Position.Y + Size.Y);

				glTexCoord2f(1, 0);
				glVertex2f(Position.X + Size.X, Position.Y - Size.Y);

				glTexCoord2f(0, 0);
				glVertex2f(Position.X - Size.X, Position.Y - Size.Y);
			glEnd();

			glDepthMask(GL_TRUE);
		}

		~ScreenQuad()
		{

		}
	};

}


