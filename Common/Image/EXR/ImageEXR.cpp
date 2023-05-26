#include <Common/Image/EXR/ImageEXR.h>
#include <System/File.h>
#include <System/Log.h>

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
		float* tmp = nullptr;
		int w, h;
		int Result = LoadEXR(&tmp, &w, &h, Filename, &Error);

		// copy with 'new', because tinyexr allocates memory with 'malloc',
		// my code frees data with delete, that's undefined behavour, so
		// I wrote that shit, lol
		size_t size = w * h * 4 * sizeof(float);
		Data = new uint8[size];
		memcpy(Data, tmp, size);
		free(tmp);

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


