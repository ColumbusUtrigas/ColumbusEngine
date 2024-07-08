#pragma once

#include <Graphics/Device.h>

namespace Columbus::Editor
{

	class Grid
	{
	private:
		BlendState* BS;
		DepthStencilState* DSS;
		RasterizerState* RS;

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
