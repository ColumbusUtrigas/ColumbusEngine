/************************************************
*              		  Texture.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <cstdio>
#include <FreeImage.h>
#include <GL/glew.h>
#include <SDL.h>
#include <cstring>

#include <System/System.h>

namespace C
{

	//Load image from file
	char* C_LoadImage(const char* aPath, int* aWidth, int* aHeight);

	enum C_TextureWrap
	{
		C_TEXTURE_CLAMP,
		C_TEXTURE_REPEAT,
		C_TEXTURE_MIRROR_REPEAT
	};

	struct C_TextureConfig
	{
		bool smooth = true;
		bool mipmaps = true;
		unsigned int anisotropy = 8;
		int wrapX = C_TEXTURE_CLAMP;
		int wrapY = C_TEXTURE_CLAMP;
	};

	class C_Texture
	{
	private:
		FIBITMAP* mBuffer = NULL;
		GLuint mID;

		char* mFile = NULL;

		bool mSmooth = true;
		bool mMipmaps = true;
		unsigned int mAnisotropy = 8;
		int mWrapX = C_TEXTURE_CLAMP;
		int mWrapY = C_TEXTURE_CLAMP;

		size_t mWidth = 0;
		size_t mHeight = 0;
		int mBPP = 0;
	public:
		//Constructor
		C_Texture();
		//Constructor 2
		C_Texture(const char* aPath, bool aSmooth = true);
		//Constructor 3
		C_Texture(const char* aData, const int aW, const int aH, bool aSmooth = true);
		//Load textures from file
		void load(const char* aPath, bool aSmooth = true);
		//Load texture from memory
		void loadFromMemory(const char* aData, size_t aSize, bool aSmooth = true);
		//Load texture from raw data
		void load(const char* aData, const int aW, const int aH, bool aSmooth = true);
		//Reload texture
		void reload();
		//Set texture smooth
		inline void setSmooth(bool aSmooth) { mSmooth = aSmooth; }
		//Set texture mipmapping
		inline void setMipmaps(bool aMipmaps) { mMipmaps = aMipmaps; }
		//Set texture anisotropy filter
		inline void setAnisotropy(unsigned int aAnisotropy) { mAnisotropy = aAnisotropy; }
		//Set texture x-wrapping
		inline void setWrapX(C_TextureWrap aWrap) { mWrapX = aWrap; }
		//Set texture y-wrapping
		inline void setWrapY(C_TextureWrap aWrap) { mWrapY = aWrap; }
		//Set texture wrapping
		inline void setWrap(C_TextureWrap aWrapX, C_TextureWrap aWrapY)
		{
			mWrapX = aWrapX;
			mWrapY = aWrapY;
		}
		//Set texture config
		void setConfig(C_TextureConfig aConfig);



		//Return texture smooth
		inline bool isSmooth() { return mSmooth; }
		//Return texture mipmapping
		inline bool isMipmaps() { return mMipmaps; }
		//Return texture anisotropy filter
		inline unsigned int getAnisotropy() { return mAnisotropy; }
		//Return texture config
		C_TextureConfig getConfig();
		//Return texture size
		size_t getSize();
		//Save texture to file
		void save(const char* aFile);
		//Bind texture
		void bind();
		//Unbind texture
		static void unbind();
		//Get texture id
		inline int getID() { return mID; }
		//Create sampler and bind texture
		void sampler2D(int a);
		//Destructor
		~C_Texture();
	};

}
