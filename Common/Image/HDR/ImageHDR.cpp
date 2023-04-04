#include <Common/Image/HDR/ImageHDR.h>
#include <System/File.h>
#include <System/Log.h>
#include <stb_image.h>

namespace Columbus
{

	bool ImageLoaderHDR::IsHDR(const char* FileName)
	{
		return stbi_is_hdr(FileName);
	}

	bool ImageLoaderHDR::Load(const char* FileName)
	{
		int x, y, comp;
		Data = (uint8*)stbi_loadf(FileName, &x, &y, &comp, 0);

		switch (comp)
		{
		case 3: Format = TextureFormat::RGB16F; break;
		case 4: Format = TextureFormat::RGBA16F; break;
		default:
			Log::Error("%s:%i:   Invalid HDR image format", __FILE__, __LINE__);
			stbi_image_free(Data);
			return false;
			break;
		}

		Width = x;
		Height = y;

		ImageType = ImageLoader::Type::Image2D;

		return Data != nullptr;
	}

}


