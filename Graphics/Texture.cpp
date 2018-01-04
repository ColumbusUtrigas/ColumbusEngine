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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <SDL_image.h>

#include <Common/Image/Image.h>

namespace Columbus
{

	static bool SDL_IMAGE_INITED = false;

	//////////////////////////////////////////////////////////////////////////////
	//Load image from file
	C_TextureData C_LoadImage(std::string aPath)
	{
		if (SDL_IMAGE_INITED == false)
		{
			IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP);
			SDL_IMAGE_INITED = true;
		}

		C_TextureData ret;

		if (aPath.substr(aPath.size() - 4) == ".bmp")
		{
			ret.buffer = ImageLoadBMP(aPath, &ret.width, &ret.height, (unsigned int*)&ret.bpp);
			return ret;
		}

		if (aPath.substr(aPath.size() - 4) == ".png")
		{
			ret.buffer = ImageLoadPNG(aPath, &ret.width, &ret.height, (unsigned int*)&ret.bpp);
			return ret;
		}

		if (ImageIsTGA(aPath))
		{
			ret.buffer = ImageLoadTGA(aPath, &ret.width, &ret.height, (unsigned int*)&ret.bpp);
			return ret;
		}

		SDL_Surface* surf = IMG_Load(aPath.c_str());

		ret.width = surf->w;
		ret.height = surf->h;
		ret.bpp = surf->format->BytesPerPixel;

		size_t size = ret.width * ret.height * ret.bpp;

		ret.buffer = new uint8_t[size];
		memcpy(ret.buffer, surf->pixels, size + 1);

		SDL_FreeSurface(surf);

		return ret;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Save image to file
	bool C_SaveImage(std::string aPath, C_TextureData aData, int aQuality)
	{
		if (aPath.empty())
			return false;
		if (aData.buffer == nullptr)
			return false;

		switch (aData.bpp)
		{
		case 3:
			stbi_write_jpg((aPath + ".jpg").c_str(), aData.width, aData.height, aData.bpp, 
				aData.buffer, aQuality);
			break;
		case 4:
			stbi_write_png((aPath + ".png").c_str(), aData.width, aData.height, aData.bpp,
				aData.buffer, aData.width * aData.bpp);
			break;
		default:
			return false;
			break;
		}

		return true;
	}
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
		mData = C_LoadImage(aPath);

		C_BindTextureOpenGL(C_OGL_TEXTURE_2D, mID);

		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_WRAP_S, C_OGL_REPEAT);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_WRAP_T, C_OGL_REPEAT);

		unsigned int format = C_OGL_RGBA;
		if (mData.bpp == 3)
			format = C_OGL_RGB;

		C_Texture2DOpenGL(C_OGL_TEXTURE_2D, 0, format, mData.width, mData.height,
			format, C_OGL_UNSIGNED_BYTE, mData.buffer);

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
		if (mData.buffer == nullptr)
			return 0;

		return mWidth * mHeight * (mBPP / 8);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Save image to file
	bool C_Texture::save(std::string aFile, int aQuality)
	{
		if (mData.buffer == nullptr)
			return false;
		
		if (C_SaveImage(aFile, mData, aQuality) == false)
		{
			C_Log::error("Can't save texture: " + aFile);
			return false;
		}

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
		free(mData.buffer);
	}

}
