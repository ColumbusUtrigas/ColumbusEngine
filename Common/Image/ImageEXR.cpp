#include <Common/Image/Image.h>
#include <System/File.h>
#include <System/Log.h>

#include <Lib/tinyexr/tinyexr.h>

namespace Columbus::ImageUtils
{

	bool ImageCheckFormatFromStreamEXR(DataStream& Stream)
	{
		if (Stream.GetType() == DataStreamType::File)
		{
			return IsEXR(Stream.F.GetName()) == TINYEXR_SUCCESS;
		}
		else
		{
			return IsEXRFromMemory(Stream.Memory, Stream.MemorySize) == TINYEXR_SUCCESS;
		}
	}

	bool ImageLoadFromStreamEXR(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData)
	{
		const char* Error = nullptr;
		float* tmp = nullptr;
		int w, h;
		int Result;

		if (Stream.GetType() == DataStreamType::File)
		{
			Result = LoadEXR(&tmp, &w, &h, Stream.F.GetName(), &Error);
		}
		else
		{
			Result = LoadEXRFromMemory(&tmp, &w, &h, Stream.Memory, Stream.MemorySize, &Error);
		}

		// copy with 'new', because tinyexr allocates memory with 'malloc',
		// my code frees data with delete, that's undefined behavour, so
		// I wrote that shit, lol
		size_t size = w * h * 4 * sizeof(float);
		OutData = new uint8[size];
		memcpy(OutData, tmp, size);
		free(tmp);

		if (Error != nullptr)
		{
			Log::Error(Error);
			FreeEXRErrorMessage(Error);
		}

		if (Result == TINYEXR_SUCCESS)
		{
			OutWidth = w;
			OutHeight = h;
			OutMips = 1;
			OutFormat = TextureFormat::RGBA32F;
			OutType = ImageType::Image2D;
			return true;
		}

		return false;
	}

}
