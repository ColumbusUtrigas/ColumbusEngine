/************************************************
*                   Cubemap.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <vector>
#include <string>
#include <GL/glew.h>
#include <FreeImage.h>

#include <Graphics/Texture.h>
#include <System/Console.h>

namespace C
{

	#define C_CubemapPath std::vector<std::string>

	class C_Cubemap
	{
	private:
		GLuint mID;
	public:
		//Constructor
		C_Cubemap(C_CubemapPath aPath);
		//Bind cubemap
		void bind();
		//Create sampler and bind cubemap
		void samplerCube(int i);
		//Unbind cubemap
		static void unbind();
		//Destructor
		~C_Cubemap();
	};

}
