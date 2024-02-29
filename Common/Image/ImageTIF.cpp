#include <Common/Image/Image.h>
#include <System/File.h>
#include <Common/Image/ImageInternalCommon.h>
#include <cstdlib>
#include <cstring>

//#include <tiff.h>
//#include <tiffio.h>

namespace Columbus::ImageUtils
{

	bool ImageCheckFormatFromStreamTIF(DataStream& Stream)
	{
		u8 magic[4];
		Stream.ReadBytes(magic, sizeof(magic));
		Stream.SeekSet(0); // rewind

		bool II = (magic[0] == 'I' &&
			magic[1] == 'I' &&
			magic[2] == 42 &&
			magic[3] == 0);

		bool MM = (magic[0] == 'M' &&
			magic[1] == 'M' &&
			magic[2] == 0 &&
			magic[3] == 42);

		if (II || MM)
		{
			return true;
		}

		return false;
	}

	bool ImageLoadFromStreamTIF(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData)
	{
		stbi_io_callbacks Callbacks = StreamToStbCallbacks(Stream);

		int x, y, chans;
		OutData = (u8*)stbi_load_from_callbacks(&Callbacks, &Stream, &x, &y, &chans, 0);
		OutWidth = x;
		OutHeight = y;
		OutMips = 1;
		OutType = ImageType::Image2D;

		switch (chans)
		{
		case 1: OutFormat = TextureFormat::R8; break;
		case 3: OutFormat = TextureFormat::RGB8; break;
		case 4: OutFormat = TextureFormat::RGBA8; break;
		}

		return OutData != nullptr;

		/*TIFF* tif = TIFFOpen(FileName, "r");
		if (tif == nullptr) return nullptr;

		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int bpp = 0;

		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &bpp);

		OutWidth = width;
		OutHeight = height;
		OutSize = width * height * bpp;

		switch (bpp)
		{
		case 1: OutFormat = TextureFormat::R8;    break;
		case 3: OutFormat = TextureFormat::RGB8;  break;
		case 4: OutFormat = TextureFormat::RGBA8; break;
		}

		uint32* buffer = (uint32*)malloc(width * height * sizeof(uint32));
		TIFFReadRGBAImage(tif, width, height, buffer, 0);

		uint8* data = new uint8[width * height * bpp];

		for (size_t i = 0; i < width * height * bpp; i += bpp)
		{
			data[i + 0] = TIFFGetR(*buffer);
			data[i + 1] = TIFFGetG(*buffer);
			data[i + 2] = TIFFGetB(*buffer);
			if (bpp == 4) data[i + 3] = TIFFGetA(*buffer);

			buffer++;
		}

		buffer -= width * height;
		free(buffer);

		TIFFClose(tif);

		ImageFlipY(data, width, height, bpp);

		return data;*/
	}
	
	bool ImageSaveToFileTIF(const char* FileName, u32 Width, u32 Height, TextureFormat Format, u8* Data)
	{
		COLUMBUS_ASSERT(false && "Not implemented");
		return false;
		/*if (Data == nullptr) return false;

		TIFF* tif = TIFFOpen(FileName, "w");
		if (tif == nullptr) return false;

		uint32 BPP = GetBPPFromFormat(Format);

		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, Width);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, Height);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, BPP);
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

		size_t stride = Width * BPP;

		uint8* row = (uint8*)malloc(stride);
		uint32 i;

		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, stride));

		for (i = 0; i < Height; i++)
		{
			memcpy(row, &Data[(Height - i - 1) * stride], stride);
			if (TIFFWriteScanline(tif, row, i, 0) < 0) break;
		}

		TIFFClose(tif);
		if (row != nullptr)
		{
			free(row);
		}

		return true;*/
	}

}
