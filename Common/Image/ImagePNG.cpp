#include <Common/Image/Image.h>
#include <System/File.h>
#include <cstdlib>
#include <cstring>
//#include <png.h>
#include <stb_image.h>
#include <Common/Image/ImageInternalCommon.h>

namespace Columbus::ImageUtils
{

	bool ImageCheckFormatFromStreamPNG(DataStream& Stream)
	{
		u8 magic[4];
		Stream.ReadBytes(magic, sizeof(magic));
		Stream.SeekSet(0); // rewind

		if (magic[1] == 'P' &&
			magic[2] == 'N' &&
			magic[3] == 'G')
		{
			return true;
		}

		return false;
	}

	bool ImageLoadFromStreamPNG(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData)
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

		/*FILE* fp = fopen(FileName, "rb");
		if (fp == nullptr) return nullptr;

		png_structp png_ptr;
		png_infop info_ptr;
		png_uint_32 width;
		png_uint_32 height;
		png_uint_32 bpp;
		int bit_depth;
		int color_type;
		int interlace_method;
		int compression_method;
		int filter_method;

		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) { fclose(fp); return nullptr; }

		info_ptr = png_create_info_struct(png_ptr);
		if (!png_ptr) { fclose(fp); return nullptr; }

		png_init_io(png_ptr, fp);
		png_read_png(png_ptr, info_ptr, 0, 0);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_method, &compression_method, &filter_method);

		switch (color_type)
		{
		case PNG_COLOR_TYPE_GRAY: bpp = 1; break;
		case PNG_COLOR_TYPE_RGB:  bpp = 3; break;
		case PNG_COLOR_TYPE_RGBA: bpp = 4; break;
		default: bpp = 0; break;
		}

		OutWidth = width;
		OutHeight = height;
		OutSize = width * height * bpp;

		switch (bpp)
		{
		case 1:  OutFormat = TextureFormat::R8;      break;
		case 3:  OutFormat = TextureFormat::RGB8;    break;
		case 4:  OutFormat = TextureFormat::RGBA8;   break;
		default: OutFormat = TextureFormat::Unknown; break;
		}

		png_bytepp rows = png_get_rows(png_ptr, info_ptr);
		int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
		uint8* data = new uint8[OutSize];

		for (size_t i = 0; i < height; i++)
		{
			memcpy(&data[rowbytes * i], rows[i], rowbytes);
		}
		
		if (png_ptr && info_ptr) png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);

		return data;*/
	}

	bool ImageSaveToFilePNG(const char* FileName, u32 Width, u32 Height, TextureFormat Format, u8* Data)
	{
		return false;
		/*FILE* fp = fopen(FileName, "wb");
		if (fp == nullptr) return false;

		png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if (!png) { fclose(fp); return false; }

		png_infop info = png_create_info_struct(png);
		if (!info)
		{
			png_destroy_write_struct(&png, &info);
			fclose(fp);
			return false;
		}

		png_init_io(png, fp);

		uint32 BPP = GetBPPFromFormat(Format);

		int type = PNG_COLOR_TYPE_RGB;
		if (BPP == 4) type = PNG_COLOR_TYPE_RGBA;

		png_set_IHDR(png, info, Width, Height, 8, type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

		png_colorp palette = (png_colorp)png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
		if (!palette)
		{
			fclose(fp);
			png_destroy_write_struct(&png, &info);
			return false;
		}

		png_set_PLTE(png, info, palette, PNG_MAX_PALETTE_LENGTH);
		png_write_info(png, info);
		png_set_packing(png);

		png_write_image(png, &Data);
		png_write_end(png, info);
		png_free(png, palette);
		png_destroy_write_struct(&png, &info);
		fclose(fp);

		return true;*/
	}

}
