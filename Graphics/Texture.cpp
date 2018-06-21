#include <Graphics/Texture.h>
#include <Common/Image/Image.h>

namespace Columbus
{

	TextureConfig Texture::GetConfig() const
	{
		return mConfig;
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
	
	Texture::~Texture()
	{

	}

}
