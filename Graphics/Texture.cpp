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

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	//Load image from file
	char* C_LoadImage(std::string aPath, int* aWidth, int* aHeight)
	{
		FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(aPath.c_str(), 0);
		if (formato == FIF_UNKNOWN) { C_Log::error("Can't get type of File: " + aPath); return NULL; }
		FIBITMAP* imagen = FreeImage_Load(formato, aPath.c_str());
		if (!imagen) { C_Log::error("Can't load Image File: " + aPath); return NULL; }
		FIBITMAP* temp = FreeImage_ConvertTo32Bits(imagen);
		if (!imagen) { C_Log::error("Can't convert image to 32 Bits: " + aPath); return NULL; }
		FreeImage_Unload(imagen);
		imagen = temp;

		char* bits = (char*)FreeImage_GetBits(imagen);

		*aWidth = FreeImage_GetWidth(imagen);
		*aHeight = FreeImage_GetHeight(imagen);

		C_Log::success("Image loaded: " + aPath);

		return bits;
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
		//if (mBuffer != NULL)
			//FreeImage_Unload(mBuffer);
		FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(aPath.c_str(), 0);
		if (formato == FIF_UNKNOWN) { C_Log::error("Can't get type of File: " + aPath); return; }
		FIBITMAP* imagen = FreeImage_Load(formato, aPath.c_str());
		if (!imagen) { C_Log::error("Can't load Image File: " + aPath); return; }
		FIBITMAP* temp = FreeImage_ConvertTo32Bits(imagen);
		if (!imagen) { C_Log::error("Can't convert image to 32 Bits: " + aPath); return; }
		FreeImage_Unload(imagen);
		imagen = temp;

		mBitmap = imagen;

		mBuffer = FreeImage_GetBits(imagen);

		char* bits = (char*)FreeImage_GetBits(imagen);
		int nWidth = FreeImage_GetWidth(imagen);
		int nHeight = FreeImage_GetHeight(imagen);
		int nBPP = FreeImage_GetBPP(imagen);

		C_BindTextureOpenGL(C_OGL_TEXTURE_2D, mID);

		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_WRAP_S, C_OGL_REPEAT);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_WRAP_T, C_OGL_REPEAT);

		C_Texture2DOpenGL(C_OGL_TEXTURE_2D, 0, C_OGL_RGBA, nWidth, nHeight,
			C_OGL_BGRA, C_OGL_UNSIGNED_BYTE, bits);

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

		mFile = aPath;
		mWidth = nWidth;
		mHeight = nHeight;
		mBPP = nBPP;

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
	//Load texture from memory
	void C_Texture::loadFromMemory(const char* aData, size_t aSize, bool aSmooth)
	{
		BYTE* mem_buffer = (BYTE*)malloc(aSize);

		memcpy(mem_buffer, aData, aSize);

		FIMEMORY* hmem = FreeImage_OpenMemory(mem_buffer, aSize);

		FIBITMAP* bitmap = FreeImage_LoadFromMemory(FreeImage_GetFileTypeFromMemory(hmem, 0), hmem, 0);
		FIBITMAP *pImage = FreeImage_ConvertTo32Bits(bitmap);

		C_BindTextureOpenGL(C_OGL_TEXTURE_2D, mID);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_WRAP_S, C_OGL_CLAMP_TO_EDGE);
		C_TextureParameterOpenGL(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_WRAP_T, C_OGL_CLAMP_TO_EDGE);
		C_Texture2DOpenGL(C_OGL_TEXTURE_2D, 0, C_OGL_RGBA, FreeImage_GetWidth(pImage),
		FreeImage_GetHeight(pImage), C_OGL_BGRA, C_OGL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(pImage));
		if (mConfig.mipmaps)
			C_GenMipmapOpenGL(C_OGL_TEXTURE_2D);

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

		C_BindTextureOpenGL(C_OGL_TEXTURE_2D, 0);

		FreeImage_CloseMemory(hmem);
		FreeImage_Unload(bitmap);
		FreeImage_Unload(pImage);
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
		if (mBuffer == nullptr)
			return 0;

		return mWidth * mHeight * (mBPP / 8);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Save image to file
	void C_Texture::save(std::string aFile)
	{
		if (mBuffer == nullptr)
			return;

		FIBITMAP* Image = FreeImage_ConvertFromRawBits(mBuffer, mWidth, mHeight,
			mWidth * (mBPP / 8), mBPP, 0xFF0000, 0x00FF00, 0x0000FF, false);
		FreeImage_Save(FIF_PNG, Image, aFile.c_str(), PNG_Z_BEST_SPEED);

		FreeImage_Unload(Image);

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
		FreeImage_Unload(mBitmap);
	}

}
