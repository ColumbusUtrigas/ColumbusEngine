#pragma once

#include <Graphics/Device.h>

namespace Columbus::Editor
{

	class Grid
	{
	private:
		Buffer* GridVertices;
		InputLayout Layout;
		uint32 VAO;
		uint32 VerticesCount;
	public:
		Grid();
		void Draw();
		~Grid();
	};

}
