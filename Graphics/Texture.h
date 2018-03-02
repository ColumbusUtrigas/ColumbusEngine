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

#include <System/Assert.h>
#include <System/System.h>
#include <System/Log.h>
#include <Common/Image/Image.h>
#include <RenderAPI/APIOpenGL.h>
#include <Math/Vector2.h>

namespace Columbus
{

	struct C_TextureConfig
	{
		bool smooth = true;
		bool mipmaps = true;
		unsigned int anisotropy = 8;
		unsigned int LOD = 0;
		C_Vector2 tiling = C_Vector2(1, 1);
		C_Vector2 tilingOffset = C_Vector2(0, 0);
	};

	class C_Texture
	{
	protected:
		C_Image mImage;

		//unsigned int mID = 0;

		std::string mFile;

		C_TextureConfig mConfig;

		size_t mWidth = 0;
		size_t mHeight = 0;
		int mBPP = 0;
	public:
		C_Texture();
		C_Texture(std::string aPath, bool aSmooth = true);
		C_Texture(const char* aData, const int aW, const int aH, bool aSmooth = true);
		
		virtual void load(std::string aPath, bool aSmooth = true);
		virtual void load(const char* aData, const int aW, const int aH, bool aSmooth = true);
		virtual void loadDepth(const char* aData, const int aW, const int aH, bool aSmooth = true);
		
		virtual void setConfig(C_TextureConfig aConfig);
		virtual void setSmooth(const bool aSmooth);
		virtual void setAnisotropy(const unsigned int aAnisotropy);

		C_TextureConfig getConfig();
		size_t getSize();

		bool save(std::string aFile, size_t aQuality = 100);
		
		virtual void bind();
		virtual void unbind();
		
		//inline unsigned int getID() { return mID; }

		virtual void sampler2D(int a);
		virtual void generateMipmap();

		virtual std::string getType();
		
		~C_Texture();
	};

}
