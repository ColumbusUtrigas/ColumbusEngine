#pragma once

#include <vector>
#include <string>
#include <GL/glew.h>
#include <FreeImage.h>

#include <Graphics/Texture.h>

namespace C
{

	#define C_CubemapPath std::vector<std::string>

	class C_Cubemap
	{
	private:
		GLuint mID;
	public:
		C_Cubemap(C_CubemapPath aPath);

		void bind();

		void samplerCube(int i);

		static void unbind();

		~C_Cubemap();
	};

}
