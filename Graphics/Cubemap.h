/************************************************
*                   Cubemap.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <array>
#include <vector>
#include <string>
#include <GL/glew.h>

#include <RenderAPI/APIOpenGL.h>
#include <Graphics/Texture.h>
#include <System/Log.h>
#include <System/Serializer.h>

namespace Columbus
{

	#define C_CubemapPath std::vector<std::string>

	class C_Cubemap
	{
	private:
		C_Image mBitmaps[6];
		unsigned int mID;

		bool mInited = false;
	public:
		//Constructor
		C_Cubemap(std::array<std::string, 6> aPath);
		//Constructor 2
		C_Cubemap(std::string aPath);
		//Bind cubemap
		void bind();
		//Create sampler and bind cubemap
		void samplerCube(int i);
		//Unbind cubemap
		static void unbind();
		//Load cubemap from 6 textures
		bool load(std::array<std::string, 6> aPath);
		//Load cubemap from 1 XML file, pointing on 6 textures
		bool load(std::string aFile);
		//Save cubemap to 6 textures
		bool save(std::array<std::string, 6> aPath);
		//Destructor
		~C_Cubemap();
	};

}
