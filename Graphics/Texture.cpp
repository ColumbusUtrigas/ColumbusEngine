/************************************************
*                  Texture.cpp                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Texture.h>
#include <Common/Image/Image.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Texture::C_Texture() :
		mBuffer(nullptr),
		mID(0),
		mWidth(0),
		mHeight(0),
		mBPP(0)
	{
		C_GenTextureOpenGL(&mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 2
	C_Texture::C_Texture(std::string aPath, bool aSmooth) :
		mBuffer(nullptr),
		mID(0),
		mWidth(0),
		mHeight(0),
		mBPP(0)
	{
		C_GenTextureOpenGL(&mID);
		load(aPath, aSmooth);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 3
	C_Texture::C_Texture(const char* aData, const int aW, const int aH, bool aSmooth) :
		mBuffer(nullptr),
		mID(0),
		mWidth(0),
		mHeight(0),
		mBPP(0)
	{
		C_GenTextureOpenGL(&mID);
		load(aData, aW, aH, aSmooth);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Load texture from file
	void C_Texture::load(std::string aPath, bool aSmooth)
	{
		mImage.load(aPath);

		C_BindTextureOpenGL(C_OGL_TEXTURE_2D, mID);

		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_WRAP_S, C_OGL_REPEAT);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_WRAP_T, C_OGL_REPEAT);

		unsigned int format = C_OGL_RGBA;
		if (mImage.getBPP() == 3)
			format = C_OGL_RGB;

		C_Texture2DOpenGL(C_OGL_TEXTURE_2D, 0, format, mImage.getWidth(), mImage.getHeight(),
			format, C_OGL_UNSIGNED_BYTE, mImage.getData());

		if (mConfig.mipmaps)
		{
			C_GenMipmapOpenGL(C_OGL_TEXTURE_2D);
			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_BASE_LEVEL, mConfig.LOD);
			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAX_LEVEL, 9);
		}

		unsigned int filter;

		if (mConfig.smooth)
		{
			mConfig.mipmaps ? (filter = C_OGL_LINEAR_MIPMAP_LINEAR) : (filter = C_OGL_LINEAR);

			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, filter);
			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, C_OGL_LINEAR);
		}
		else
		{
			mConfig.mipmaps ? (filter = C_OGL_NEAREST_MIPMAP_NEAREST) : (filter = C_OGL_NEAREST);

			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, filter);
			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, C_OGL_NEAREST);
		}

		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAX_ANISOTROPY, mConfig.anisotropy);

		C_BindTextureOpenGL(C_OGL_TEXTURE_2D, 0);

		C_Log::success("Texture loaded: " + aPath);
	}

	//////////////////////////////////////////////////////////////////////////////
	//Load texture from raw data
	void C_Texture::load(const char* aData, const int aW, const int aH, bool aSmooth)
	{
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		C_BindTextureOpenGL(C_OGL_TEXTURE_2D, mID);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_WRAP_S, C_OGL_CLAMP_TO_EDGE);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_WRAP_T, C_OGL_CLAMP_TO_EDGE);

		unsigned int filter;

		if (mConfig.smooth)
		{
			mConfig.mipmaps ? (filter = C_OGL_LINEAR_MIPMAP_LINEAR) : (filter = C_OGL_LINEAR);

			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, filter);
			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, C_OGL_LINEAR);
		}
		else
		{
			mConfig.mipmaps ? (filter = C_OGL_NEAREST_MIPMAP_NEAREST) : (filter = C_OGL_NEAREST);

			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, filter);
			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, C_OGL_NEAREST);
		}

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA/*GL_RED*/, aW, aH,
		//0, GL_RGBA/*GL_RED*/, GL_UNSIGNED_BYTE, (void*)aData);

		C_Texture2DOpenGL(C_OGL_TEXTURE_2D, 0, C_OGL_RGBA, aW, aH,
		C_OGL_RGBA, C_OGL_UNSIGNED_BYTE, (void*)aData);

		C_BindTextureOpenGL(C_OGL_TEXTURE_2D, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Load depth texture from raw data
	void C_Texture::loadDepth(const char* aData, const int aW, const int aH, bool aSmooth)
	{
		C_BindTextureOpenGL(C_OGL_TEXTURE_2D, mID);

		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		unsigned int filter;

		if (mConfig.smooth)
		{
			mConfig.mipmaps ? (filter = C_OGL_LINEAR_MIPMAP_LINEAR) : (filter = C_OGL_LINEAR);

			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, filter);
			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, C_OGL_LINEAR);
		}
		else
		{
			mConfig.mipmaps ? (filter = C_OGL_NEAREST_MIPMAP_NEAREST) : (filter = C_OGL_NEAREST);

			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, filter);
			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, C_OGL_NEAREST);
		}

		C_Texture2DOpenGL(C_OGL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, aW, aH,
		C_OGL_DEPTH_COMPONENT, C_OGL_FLOAT, NULL);

		C_BindTextureOpenGL(C_OGL_TEXTURE_2D, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set texture config
	void C_Texture::setConfig(C_TextureConfig aConfig)
	{
		C_TextureConfig conf = mConfig;

		mConfig = aConfig;

		if (aConfig.smooth != conf.smooth)
			setSmooth(aConfig.smooth);

		if (aConfig.anisotropy != conf.anisotropy)
			setAnisotropy(aConfig.anisotropy);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set texture smooth
	void C_Texture::setSmooth(const bool aSmooth)
	{
		if (mConfig.smooth != aSmooth)
		{
			unsigned int filter;

			if (aSmooth == true)
				mConfig.mipmaps ? (filter = C_OGL_LINEAR_MIPMAP_LINEAR) : (filter = C_OGL_LINEAR);
			else
				mConfig.mipmaps ? (filter = C_OGL_NEAREST_MIPMAP_NEAREST) : (filter = C_OGL_NEAREST);

			mConfig.smooth = (bool)aSmooth;
			bind();
			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MIN_FILTER, filter);
			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, filter);
			unbind();
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set texture anisotropy filtration value
	void C_Texture::setAnisotropy(const unsigned int aAnisotropy)
	{
		if (mConfig.anisotropy != aAnisotropy)
		{
			mConfig.anisotropy = (unsigned int)aAnisotropy;
			bind();
			C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAX_ANISOTROPY, aAnisotropy);
			unbind();
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return texture config
	C_TextureConfig C_Texture::getConfig()
	{
		return mConfig;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return texture size
	size_t C_Texture::getSize()
	{
		if (!mImage.isExist()) return 0;

		return mWidth * mHeight * (mBPP / 8);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Save image to file
	bool C_Texture::save(std::string aFile, size_t aQuality)
	{
		if (!mImage.isExist())
		{ C_Log::error("Texture didn't saved: " + aFile);  return false; }

		int type = E_IMAGE_SAVE_FORMAT_PNG;

		switch (mImage.getBPP())
		{
		case 3: type = E_IMAGE_SAVE_FORMAT_JPG; break;
		case 4: type = E_IMAGE_SAVE_FORMAT_PNG; break;
		}

		ImageSave(aFile, mImage.getWidth(), mImage.getHeight(), mImage.getBPP(), mImage.getData(), type, aQuality);

		C_Log::success("Texture successfully saved: " + aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Bind texture
	void C_Texture::bind()
	{
		if(mID != 0)
			C_BindTextureOpenGL(C_OGL_TEXTURE_2D, mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Unbind texture
	void C_Texture::unbind()
	{
		C_BindTextureOpenGL(C_OGL_TEXTURE_2D, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Create sampler and bind texture
	void C_Texture::sampler2D(int a)
	{
		C_ActiveTextureOpenGL(C_OGL_TEXTURE0 + a);
		bind();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Generate mipmap for texture
	void C_Texture::generateMipmap()
	{
		bind();
		C_GenMipmapOpenGL(C_OGL_TEXTURE_2D);
		unbind();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Texture::~C_Texture()
	{
		C_DeleteTextureOpenGL(&mID);
	}

}
