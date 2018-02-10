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

		for (int i = 0; i < 6; i++)
		{
			mBitmaps[i].load(aPath[i]);
			mBitmaps[i].flipY();

			if (!mBitmaps[i].isExist())
			{
				C_Log::error("Can't load Cubemap");
				C_DeleteTextureOpenGL(&mID);
				return false;
			}
			else
			{
				unsigned int format = C_OGL_RGBA;
				if (mBitmaps[i].getBPP() == 3)
					format = C_OGL_RGB;

				C_Texture2DOpenGL(C_OGL_TEXTURE_CUBE_MAP_POS_X + i, 0, format,
					mBitmaps[i].getWidth(), mBitmaps[i].getHeight(), format, C_OGL_UNSIGNED_BYTE,
						mBitmaps[i].getData());
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
			if (!mBitmaps[i].isExist()) return false;

			int type = E_IMAGE_SAVE_FORMAT_PNG;
			if (mBitmaps[i].getBPP() == 3)
				type == E_IMAGE_SAVE_FORMAT_JPG;

			if (!mBitmaps[i].save(aPath[i], type, 90))
			{
				C_Log::error("Can't load cubemap face: " + aPath[i]);
				return false;
			}

			C_Log::success("Cubemap face successfully saved: " + aPath[i]);
		}
		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Cubemap::~C_Cubemap()
	{
		C_DeleteTextureOpenGL(&mID);
	}

}
