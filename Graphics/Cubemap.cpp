/************************************************
*                  Cubemap.cpp                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Cubemap.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Cubemap::C_Cubemap(std::array<std::string, 6> aPath)
	{
		C_GenTextureOpenGL(&mID);

		C_BindTextureOpenGL(C_OGL_TEXTURE_CUBE_MAP, mID);

		char* data[6];
		int nWidth[6];
		int nHeight[6];

		for (int i = 0; i < 6; i++)
		{
			data[i] = C_LoadImage(aPath[i].c_str(), &nWidth[i], &nHeight[i]);
			if (data[i] == NULL)
			{
				C_Error("Can't load Cubemap");
				C_DeleteTextureOpenGL(&mID);
				return;
			}
			else
			{
				C_Texture2DOpenGL(C_OGL_TEXTURE_CUBE_MAP_POS_X + i, 0, C_OGL_RGBA,
				nWidth[i], nHeight[i], C_OGL_BGRA, C_OGL_UNSIGNED_BYTE, data[i]);
			}
		}

		C_TextureParameterOpenGL(C_OGL_TEXTURE_CUBE_MAP, C_OGL_TEXTURE_MIN_FILTER, C_OGL_LINEAR);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_CUBE_MAP, C_OGL_TEXTURE_MAG_FILTER, C_OGL_LINEAR);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_CUBE_MAP, C_OGL_TEXTURE_WRAP_S, C_OGL_CLAMP_TO_EDGE);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_CUBE_MAP, C_OGL_TEXTURE_WRAP_T, C_OGL_CLAMP_TO_EDGE);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_CUBE_MAP, C_OGL_TEXTURE_WRAP_R, C_OGL_CLAMP_TO_EDGE);

		mInited = true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Bind cubemap
	void C_Cubemap::bind()
	{
		if (!mInited)
			return;

		C_BindTextureOpenGL(C_OGL_TEXTURE_CUBE_MAP, mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Create sampler and bind cubemap
	void C_Cubemap::samplerCube(int i)
	{
		if (!mInited)
			return;

		C_ActiveTextureOpenGL(C_OGL_TEXTURE0 + i);
		bind();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Unbind cubemap
	void C_Cubemap::unbind()
	{
		C_BindTextureOpenGL(C_OGL_TEXTURE_CUBE_MAP, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Cubemap::~C_Cubemap()
	{
		C_DeleteTextureOpenGL(&mID);
	}

}
