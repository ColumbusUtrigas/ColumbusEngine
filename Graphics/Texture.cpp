#include <Graphics/Texture.h>
#include <Common/Image/Image.h>

namespace Columbus
{

	
	Texture::Texture()
	{

	}
	
	Texture::Texture(std::string aPath, bool aSmooth)
	{

	}
	
	Texture::Texture(const char* aData, const int aW, const int aH, bool aSmooth)
	{

	}
	
	void Texture::load(std::string aPath, bool aSmooth)
	{

	}

	
	void Texture::load(const char* aData, const int aW, const int aH, bool aSmooth)
	{

	}
	
	void Texture::loadDepth(const char* aData, const int aW, const int aH, bool aSmooth)
	{

	}
	
	void Texture::setConfig(TextureConfig aConfig)
	{

	}
	
	void Texture::setSmooth(const bool aSmooth)
	{

	}
	
	void Texture::setAnisotropy(const unsigned int aAnisotropy)
	{

	}
	
	TextureConfig Texture::GetConfig() const
	{
		return mConfig;
	}
	
	uint64 Texture::GetSize() const
	{
		if (!mImage.IsExist()) return 0;

		return mWidth * mHeight * (mBPP / 8);
	}
	
	bool Texture::save(std::string aFile, size_t aQuality)
	{
		if (!mImage.IsExist())
		{ Log::error("Texture didn't saved: " + aFile);  return false; }

		ImageFormat type = ImageFormat::PNG;

		switch (GetBPPFromFormat(mImage.GetFormat()))
		{
		case 3: type = ImageFormat::JPG; break;
		case 4: type = ImageFormat::PNG; break;
		}

		ImageSave(aFile, mImage.GetWidth(), mImage.GetHeight(), mImage.GetFormat(), mImage.GetData(), type, aQuality);

		Log::success("Texture successfully saved: " + aFile);
		return true;
	}
	
	void Texture::bind()
	{

	}
	
	void Texture::unbind()
	{

	}
	
	void Texture::sampler2D(int a)
	{

	}
	
	void Texture::generateMipmap()
	{

	}
	
	std::string Texture::getType()
	{
		return "Texture";
	}
	
	Texture::~Texture()
	{

	}

}
