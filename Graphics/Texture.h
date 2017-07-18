#pragma once
#include <cstdio>
#include <FreeImage.h>
#include <GL/glew.h>
#include <SDL.h>
#include <cstring>

#include <System/System.h>

namespace C
{

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
		C_Texture();
		C_Texture(const char* aPath, bool aSmooth = true);
		C_Texture(const char* aData, const int aW, const int aH, bool aSmooth = true);

		void load(const char* aPath, bool aSmooth = true);

		void loadFromMemory(const char* aData, size_t aSize, bool aSmooth = true);

		void load(const char* aData, const int aW, const int aH, bool aSmooth = true);

		void reload();

		inline void setSmooth(bool aSmooth) { mSmooth = aSmooth; }

		inline void setMipmaps(bool aMipmaps) { mMipmaps = aMipmaps; }

		inline void setAnisotropy(unsigned int aAnisotropy) { mAnisotropy = aAnisotropy; }

		inline void setWrapX(C_TextureWrap aWrap) { mWrapX = aWrap; }

		inline void setWrapY(C_TextureWrap aWrap) { mWrapY = aWrap; }

		inline void setWrap(C_TextureWrap aWrapX, C_TextureWrap aWrapY)
		{
			mWrapX = aWrapX;
			mWrapY = aWrapY;
		}

		void setConfig(C_TextureConfig aConfig);

		inline bool isSmooth() { return mSmooth; }

		inline bool isMipmaps() { return mMipmaps; }

		inline unsigned int getAnisotropy() { return mAnisotropy; }

		C_TextureConfig getConfig();

		size_t getSize();

		void save(const char* aFile);

		void bind();

		static void unbind();

		inline int getID() { return mID; }

		void sampler2D(int a);

		~C_Texture();
	};

}
