#pragma once

#include <Core/Types.h>
#include <GL/glew.h>

namespace Columbus
{

	class ScreenQuad
	{
	public:
		ScreenQuad() {}

		void Render()
		{
			glDisable(GL_CULL_FACE);
			glDepthMask(GL_FALSE);

			glBegin(GL_QUADS);
				glTexCoord2f(0, 1);
				glVertex2f(-1, 1);

				glTexCoord2f(1, 1);
				glVertex2f(1, 1);

				glTexCoord2f(1, 0);
				glVertex2f(1, -1);

				glTexCoord2f(0, 0);
				glVertex2f(-1, -1);
			glEnd();

			glDepthMask(GL_TRUE);
		}

		~ScreenQuad()
		{

		}
	};

}


