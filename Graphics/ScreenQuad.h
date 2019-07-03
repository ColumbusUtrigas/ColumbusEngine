#pragma once

#include <Math/Vector2.h>
#include <Graphics/OpenGL/BufferOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	class ScreenQuad
	{
	private:
		BufferOpenGL VB;
		BufferOpenGL IB;

		uint32 VAO = 0;

		void Init()
		{
			float Vertices[] = { -1, 1, 1, 1, 1, -1, -1, -1 };
			float UV[] = { 0, 1, 1, 1, 1, 0, 0, 0 };
			uint32 Indices[] = { 0, 1, 2, 3, 0, 2 };

			VB.CreateArray(BufferDesc(sizeof(Vertices) + sizeof(UV)));
			IB.CreateIndex(BufferDesc(sizeof(Indices)));

			VB.SubLoad(Vertices, 0, sizeof(Vertices));
			VB.SubLoad(UV, sizeof(Vertices), sizeof(UV));

			IB.Load(Indices);

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			VB.VertexAttribute<float>(0, 2, false, 0, 0);
			VB.VertexAttribute<float>(1, 2, false, 0, sizeof(Vertices));

			glBindVertexArray(0);
		}
	public:
		ScreenQuad()
		{
			Init();
		}

		void Render()
		{
			glDisable(GL_CULL_FACE);
			glDepthMask(GL_FALSE);

			glBindVertexArray(VAO);
			IB.Bind();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			IB.Unbind();
			glBindVertexArray(0);

			glDepthMask(GL_TRUE);
		}

		~ScreenQuad()
		{
			glDeleteVertexArrays(1, &VAO);
		}
	};

}


