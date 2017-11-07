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

		if (!load(aPath))
			return;

		mInited = true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 2
	C_Cubemap::C_Cubemap(std::string aPath)
	{
		C_GenTextureOpenGL(&mID);
		
		if (!load(aPath))
			return;

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
	//Load cubemap from 6 textures
	bool C_Cubemap::load(std::array<std::string, 6> aPath)
	{
		C_BindTextureOpenGL(C_OGL_TEXTURE_CUBE_MAP, mID);

		char* data[6];
		int nWidth[6];
		int nHeight[6];

		for (int i = 0; i < 6; i++)
		{
			//data[i] = C_LoadImage(aPath[i], &nWidth[i], &nHeight[i]);
			mBitmaps[i] = C_LoadImage(aPath[i]);
			data[i] = (char*)FreeImage_GetBits(mBitmaps[i]);
			nWidth[i] = FreeImage_GetWidth(mBitmaps[i]);
			nHeight[i] = FreeImage_GetHeight(mBitmaps[i]);

			if (data[i] == NULL)
			{
				C_Log::error("Can't load Cubemap");
				C_DeleteTextureOpenGL(&mID);
				return false;
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

		C_BindTextureOpenGL(C_OGL_TEXTURE_CUBE_MAP, 0);

		return true;
	}
	//Load cubemap from 1 XML file, pointing on 6 textures
	bool C_Cubemap::load(std::string aPath)
	{
		std::array<std::string, 6> pathes;
		std::string names[6] = 
		{
			"PosX", "NegX", "PosY", "NegY", "PosZ", "NegZ"
		};

		Serializer::C_SerializerXML serializer;

		if (!serializer.read(aPath, "Cubemap"))
		{ C_Log::error("Can't load Cubemap XML: " + aPath); return false; }

		for (int i = 0; i < 6; i++)
		{
			if (!serializer.getString(names[i], &pathes[i]))
			{ C_Log::error("Can't load Cubemap XML: " + aPath); return false; }
		}

		return load(pathes);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Save cubemap to 6 textures
	bool C_Cubemap::save(std::array<std::string, 6> aPath)
	{
		for (int i = 0; i < 6; i++)
		{
			if (mBitmaps[i] == nullptr)
				return false;

				FreeImage_Save(FIF_PNG, mBitmaps[i], aPath[i].c_str(), PNG_Z_BEST_SPEED);
				C_Log::success("Cubemap face successfully saved: " + aPath[i]);
		}
		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Cubemap::~C_Cubemap()
	{
		C_DeleteTextureOpenGL(&mID);
		for (int i = 0; i < 6; i++)
			FreeImage_Unload(mBitmaps[i]);
	}

}
