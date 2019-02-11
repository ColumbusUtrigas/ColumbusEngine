#include <Common/Image/Image.h>
#include <Common/Image/PNG/ImagePNG.h>
#include <Core/Memory.h>
#include <System/File.h>
#include <png.h>

namespace Columbus
{

	static uint8* ImageLoadPNG(const char* FileName, uint32& OutWidth, uint32& OutHeight, uint64& OutSize, TextureFormat& OutFormat)
	{
		FILE* fp = fopen(FileName, "rb");
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

		png_bytepp rows;

		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) { fclose(fp); return nullptr; }

		info_ptr = png_create_info_struct(png_ptr);
		if (!png_ptr) { fclose(fp); return nullptr; }

		png_init_io(png_ptr, fp);
		png_read_png(png_ptr, info_ptr, 0, 0);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_method, &compression_method, &filter_method);

		rows = png_get_rows(png_ptr, info_ptr);
		int rowbytes;
		rowbytes = png_get_rowbytes(png_ptr, info_ptr);

		if (color_type == PNG_COLOR_TYPE_GRAY) bpp = 1;
		if (color_type == PNG_COLOR_TYPE_RGB) bpp = 3;
		if (color_type == PNG_COLOR_TYPE_RGBA) bpp = 4;

		OutWidth = width;
		OutHeight = height;
		OutSize = width * height * bpp;

		switch (bpp)
		{
		case 1: OutFormat = TextureFormat::R8;    break;
		case 3: OutFormat = TextureFormat::RGB8;  break;
		case 4: OutFormat = TextureFormat::RGBA8; break;
		}

		uint8* data = (uint8*)Memory::Malloc(width * height * bpp);

		//Copying row data into byte buffer with reversed vertical
		for (size_t i = 0; i < height; i++)
		{
			Memory::Memcpy(&data[rowbytes * i], rows[height - i - 1], rowbytes);
		}
		
		if (png_ptr && info_ptr) png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);

		return data;
	}

	bool ImageSavePNG(const char* FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data)
	{
		FILE* fp = fopen(FileName, "wb");
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

		png_bytepp rows = (png_bytepp)png_malloc(png, Height * sizeof(png_bytep));
		int rowbytes = Width * BPP;
		for (size_t i = 0; i < Height; i++)
		{
			rows[i] = (png_bytep)Memory::Malloc(rowbytes);
			Memory::Memcpy(rows[i], Data + (Height - i - 1) * rowbytes, rowbytes);
		}

		png_write_image(png, rows);
		png_write_end(png, info);
		png_free(png, palette);
		png_destroy_write_struct(&png, &info);
		fclose(fp);

		for (size_t i = 0; i < Height; i++)
		{
			Memory::Free(rows[i]);
		}

		return true;
	}

	bool ImageLoaderPNG::IsPNG(const char* FileName)
	{
		File PNGImageFile(FileName, "rb");
		if (!PNGImageFile.IsOpened()) return false;

		uint8 magic[4];
		if (!PNGImageFile.ReadBytes(magic, sizeof(magic))) return false;
		PNGImageFile.Close();

		if (magic[1] == 'P' &&
		    magic[2] == 'N' &&
		    magic[3] == 'G')
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool ImageLoaderPNG::Load(const char* FileName)
	{
		uint64 Size = 0;

		Data = ImageLoadPNG(FileName, Width, Height, Size, Format);
		ImageType = ImageLoader::Type::Image2D;

		return (Data != nullptr);
	}

}


