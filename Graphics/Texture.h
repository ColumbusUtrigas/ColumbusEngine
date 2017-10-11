/************************************************
*                   Texture.h                   *
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
#include <System/Console.h>
#include <RenderAPI/APIOpenGL.h>

namespace C
{

	//Load image from file
	char* C_LoadImage(const char* aPath, int* aWidth, int* aHeight);

	struct C_TextureConfig
	{
		bool smooth = true;
		bool mipmaps = true;
		unsigned int anisotropy = 8;
		unsigned int LOD = 0;
	};

	class C_Texture
	{
	private:
		FIBITMAP* mBuffer = NULL;
		unsigned int mID;

		char* mFile = NULL;

		C_TextureConfig mConfig;

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
		//Load depth texture from raw data
		void loadDepth(const char* aData, const int aW, const int aH, bool aSmooth = true);
		//Reload texture
		void reload();
		//Set texture config
		void setConfig(C_TextureConfig aConfig);

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
		inline unsigned int getID() { return mID; }
		//Create sampler and bind texture
		void sampler2D(int a);
		//Generate mipmap for texture
		void generateMipmap();
		//Destructor
		~C_Texture();
	};

}
