#pragma once

#include <Math/Vector2.h>
#include <GL/glew.h>

namespace Columbus
{

	class ScreenQuad
	{
	private:
		void Init(uint32& VAO, uint32& IBO)
		{
			uint32 VBO;

			float Vertices[] = { -1, 1, 1, 1, 1, -1, -1, -1 };
			float UV[] = { 0, 1, 1, 1, 1, 0, 0, 0  };
			uint32 Indices[] = { 0, 1, 2, 3, 0, 2 };

			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(UV), nullptr, GL_STATIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices), sizeof(UV), UV);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glGenBuffers(1, &IBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, GL_FALSE, nullptr);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, GL_FALSE, (void*)(sizeof(Vertices)));
			glEnableVertexAttribArray(1);

			glBindVertexArray(0);
		}
	public:
		void Render()
		{
			static uint32 VAO = 0;
			static uint32 IBO = 0;

			if (VAO == 0)
			{
				Init(VAO, IBO);
			}

			glDisable(GL_CULL_FACE);
			glDepthMask(GL_FALSE);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			glDepthMask(GL_TRUE);
		}
	};

}


