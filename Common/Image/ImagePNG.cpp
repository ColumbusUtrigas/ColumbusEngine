/************************************************
*              	   ImagePNG.cpp                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   04.01.2018                  *
*************************************************/
#include <Common/Image/Image.h>
#include <System/File.h>
#include <png.h>

namespace Columbus
{

	bool ImageIsPNG(std::string aFile)
	{
		C_File file(aFile, "rb");
		if (!file.isOpened()) return false;

		uint8_t magic[4];
		if (!file.readBytes(magic, sizeof(magic))) return false;
		file.close();

		if (magic[1] == 'P' &&
		    magic[2] == 'N' &&
		    magic[3] == 'G') return true;
		else return false;
	}

	unsigned char* ImageLoadPNG(const std::string aFile, unsigned int* aWidth, unsigned int* aHeight, unsigned int* aBPP)
	{
		COLUMBUS_ASSERT_MESSAGE(aWidth, "ImageLoadPNG(): invalid width")
		COLUMBUS_ASSERT_MESSAGE(aHeight, "ImageLoadPNG(): invalid height")
		COLUMBUS_ASSERT_MESSAGE(aBPP, "ImageLoadPNG(): invalid BPP")

		FILE* fp = fopen(aFile.c_str(), "rb");
		if (fp == nullptr) return nullptr;

		png_structp	png_ptr;
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
		if (!png_ptr) return nullptr;

		info_ptr = png_create_info_struct(png_ptr);
		if (!png_ptr) return nullptr;

		png_init_io(png_ptr, fp);
		png_read_png(png_ptr, info_ptr, 0, 0);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_method, &compression_method, &filter_method);

		rows = png_get_rows(png_ptr, info_ptr);
		int rowbytes;
		rowbytes = png_get_rowbytes(png_ptr, info_ptr);

		if (color_type == PNG_COLOR_TYPE_RGB)
			bpp = 3;
		if (color_type == PNG_COLOR_TYPE_RGBA)
			bpp = 4;

		*aWidth = width;
		*aHeight = height;
		*aBPP = bpp;

		uint8_t* data = (uint8_t*)malloc(width * height * bpp);

		for (size_t i = 0; i < height; i++)
			memcpy(&data[rowbytes * i], rows[height - i - 1], rowbytes);

		if (png_ptr && info_ptr)
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

		fclose(fp);

		return data;
	}

	bool ImageSavePNG(const std::string aFile, const unsigned int aWidth, const unsigned int aHeight, const unsigned int aBPP, const unsigned char* aData)
	{
		FILE* fp = fopen(aFile.c_str(), "wb");
		if (fp == nullptr) return false;

		png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if (!png) return false;

		png_infop info = png_create_info_struct(png);
		if (!info)
		{
			png_destroy_write_struct(&png, &info);
			return false;
		}

		png_init_io(png, fp);
		int type = PNG_COLOR_TYPE_RGB;
		if (aBPP == 4)
			type = PNG_COLOR_TYPE_RGBA;

		png_set_IHDR(png, info, aWidth, aHeight, 8, type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

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

		png_bytepp rows = (png_bytepp)png_malloc(png, aHeight * sizeof(png_bytep));
		int rowbytes = aWidth * aBPP;
		for (size_t i = 0; i < aHeight; i++)
		{
			rows[i] = (png_bytep)malloc(rowbytes);
			memcpy(rows[i], aData + (aHeight - i - 1) * rowbytes, rowbytes);
		}

		png_write_image(png, rows);
		png_write_end(png, info);
		png_free(png, palette);
		png_destroy_write_struct(&png, &info);
		fclose(fp);

		for (size_t i = 0; i < aHeight; i++)
			free(rows[i]);

		return true;
	}

}


