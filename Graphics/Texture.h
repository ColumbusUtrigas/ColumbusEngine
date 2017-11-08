/************************************************
*                   Texture.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <System/System.h>
#include <System/Log.h>
#include <RenderAPI/APIOpenGL.h>
#include <Math/Vector2.h>

namespace Columbus
{

	struct C_TextureData;

	//Load image from file
	C_TextureData C_LoadImage(std::string aPath);

	struct C_TextureConfig
	{
		bool smooth = true;
		bool mipmaps = true;
		unsigned int anisotropy = 8;
		unsigned int LOD = 0;
		C_Vector2 tiling = C_Vector2(1, 1);
		C_Vector2 tilingOffset = C_Vector2(0, 0);
	};

	struct C_TextureData
	{
		unsigned char* buffer = NULL;
		size_t width = 0;
		size_t height = 0;
		int bpp = 0;
	};

	class C_Texture
	{
	private:
		C_TextureData mData;

		uint8_t* mBuffer = nullptr;
		unsigned int mID = 0;

		std::string mFile;

		C_TextureConfig mConfig;

		size_t mWidth = 0;
		size_t mHeight = 0;
		int mBPP = 0;
	public:
		//Constructor
		C_Texture();
		//Constructor 2
		C_Texture(std::string aPath, bool aSmooth = true);
		//Constructor 3
		C_Texture(const char* aData, const int aW, const int aH, bool aSmooth = true);
		//Load textures from file
		void load(std::string aPath, bool aSmooth = true);
		//Load texture from raw data
		void load(const char* aData, const int aW, const int aH, bool aSmooth = true);
		//Load depth texture from raw data
		void loadDepth(const char* aData, const int aW, const int aH, bool aSmooth = true);
		//Set texture config
		void setConfig(C_TextureConfig aConfig);
		//Set texture smooth
		void setSmooth(const bool aSmooth);
		//Set texture anisotropy filtration value
		void setAnisotropy(const unsigned int aAnisotropy);

		//Return texture config
		C_TextureConfig getConfig();
		//Return texture size
		size_t getSize();
		//Save texture to file
		void save(std::string aFile);
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
