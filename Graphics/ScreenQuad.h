#pragma once

#include <Math/Vector2.h>
#include <Graphics/OpenGL/BufferOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	class ScreenQuad
	{
	private:
		uint32 VAO = 0;
	public:
		ScreenQuad()
		{
			glGenVertexArrays(1, &VAO);
		}

		void Render()
		{
			glDisable(GL_CULL_FACE);
			glDepthMask(GL_FALSE);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
			glDepthMask(GL_TRUE);
		}

		~ScreenQuad()
		{
			glDeleteVertexArrays(1, &VAO);
		}
	};

}


