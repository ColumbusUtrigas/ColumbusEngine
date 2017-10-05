/************************************************
*                  Cubemap.cpp                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Cubemap.h>

namespace C
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Cubemap::C_Cubemap(C_CubemapPath aPath)
	{
		glGenTextures(1, &mID);

		glBindTexture(GL_TEXTURE_CUBE_MAP, mID);

		char* data[6];
		int nWidth[6];
		int nHeight[6];

		for (int i = 0; i < 6; i++)
		{
			data[i] = C_LoadImage(aPath[i].c_str(), &nWidth[i], &nHeight[i]);
			if (data[i] == NULL) { C_Error("Can't load Cubemap"); glDeleteTextures(1, &mID); return; }
			else
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, nWidth[i], nHeight[i], 0, GL_BGRA, GL_UNSIGNED_BYTE, data[i]);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Bind cubemap
	void C_Cubemap::bind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Create sampler and bind cubemap
	void C_Cubemap::samplerCube(int i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		bind();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Unbind cubemap
	void C_Cubemap::unbind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Cubemap::~C_Cubemap()
	{
		glDeleteTextures(1, &mID);
	}

}
