#pragma once

#include <System/Assert.h>
#include <System/System.h>
#include <System/Log.h>
#include <Common/Image/Image.h>
#include <RenderAPI/APIOpenGL.h>
#include <Math/Vector2.h>

namespace Columbus
{

	struct TextureConfig
	{
		bool smooth = true;
		bool mipmaps = true;
		unsigned int anisotropy = 8;
		unsigned int LOD = 0;
		Vector2 tiling = Vector2(1, 1);
		Vector2 tilingOffset = Vector2(0, 0);
	};

	class Texture
	{
	protected:
		Image mImage;

		std::string mFile;

		TextureConfig mConfig;

		size_t mWidth = 0;
		size_t mHeight = 0;
		uint32 mBPP = 0;
	public:
		Texture();
		Texture(std::string aPath, bool aSmooth = true);
		Texture(const char* aData, const int aW, const int aH, bool aSmooth = true);
		
		virtual void load(std::string aPath, bool aSmooth = true);
		virtual void load(const char* aData, const int aW, const int aH, bool aSmooth = true);
		virtual void loadDepth(const char* aData, const int aW, const int aH, bool aSmooth = true);
		
		virtual void setConfig(TextureConfig aConfig);
		virtual void setSmooth(const bool aSmooth);
		virtual void setAnisotropy(const unsigned int aAnisotropy);

		TextureConfig GetConfig() const;
		uint64 GetSize() const;

		bool save(std::string aFile, size_t aQuality = 100);
		
		virtual void bind();
		virtual void unbind();

		virtual void sampler2D(int a);
		virtual void generateMipmap();

		virtual std::string getType();
		
		virtual ~Texture();
	};

}
