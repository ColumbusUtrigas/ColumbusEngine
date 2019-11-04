#pragma once

#include <Graphics/OpenGL/BufferOpenGL.h>

namespace Columbus
{

	class Grid
	{
	private:
		BufferOpenGL GridVertices;
		uint32 VAO;
		uint32 VerticesCount;
	public:
		Grid();

		void Draw();

		~Grid();
	};

}


