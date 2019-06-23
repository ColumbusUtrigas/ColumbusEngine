#include <Common/Image/EXR/ImageEXR.h>
#include <System/File.h>
#include <System/Log.h>

#define TINYEXR_IMPLEMENTATION
#include <Lib/tinyexr/tinyexr.h>

namespace Columbus
{

	static bool ImageEXR_Check(const char* Filename)
	{
		return IsEXR(Filename) == TINYEXR_SUCCESS;
	}

	bool ImageLoaderEXR::IsEXR(const char* Filename)
	{
		return ImageEXR_Check(Filename);
	}

	bool ImageLoaderEXR::Load(const char* Filename)
	{
		const char* Error = nullptr;
		int w, h;
		int Result = LoadEXR((float**)&Data, &w, &h, Filename, &Error);

		if (Error != nullptr)
		{
			Log::Error(Error);
			FreeEXRErrorMessage(Error);
		}

		if (Result == TINYEXR_SUCCESS)
		{
			Width = w;
			Height = h;
			Format = TextureFormat::RGBA32F;
			ImageType = ImageLoader::Type::Image2D;
			return true;
		}

		return false;
	}

}


