#include <Common/Image/Image.h>
#include <System/File.h>
#include <System/Log.h>
#include <stb_image.h>
#include <Common/Image/ImageInternalCommon.h>

namespace Columbus::ImageUtils
{

	bool ImageCheckFormatFromStreamHDR(DataStream& Stream)
	{
		stbi_io_callbacks Callbacks = StreamToStbCallbacks(Stream);

		int Result = stbi_is_hdr_from_callbacks(&Callbacks, &Stream);
		Stream.SeekSet(0); // rewind
		
		return Result;
	}

	bool ImageLoadFromStreamHDR(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData)
	{
		stbi_io_callbacks Callbacks = StreamToStbCallbacks(Stream);

		int x, y, comp;
		OutData = (u8*)stbi_loadf_from_callbacks(&Callbacks, &Stream, &x, &y, &comp, 0);

		switch (comp)
		{
		case 3: OutFormat = TextureFormat::RGB16F; break;
		case 4: OutFormat = TextureFormat::RGBA16F; break;
		default:
			Log::Error("%s:%i:   Invalid HDR image format", __FILE__, __LINE__);
			stbi_image_free(OutData);
			return false;
			break;
		}

		OutWidth = x;
		OutHeight = y;
		OutMips = 1;
		OutType = ImageType::Image2D;

		return OutData != nullptr;
	}

}
