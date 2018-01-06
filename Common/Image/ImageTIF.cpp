/************************************************
*              	   ImageTIF.cpp                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   06.01.2018                  *
*************************************************/
#include <Common/Image/Image.h>
#include <System/File.h>
#include <tiff.h>
#include <tiffio.h>

namespace Columbus
{

	bool ImageIsTIF(std::string aFile)
	{
		C_File file(aFile, "rb");
		if (!file.isOpened()) return false;

		uint8_t magic[4];
		if (!file.readBytes(magic, sizeof(magic))) return false;
		file.close();

		bool II = (magic[0] == 'I' &&
			       magic[1] == 'I' &&
		           magic[2] == 42 &&
		           magic[3] == 0);

		bool MM = (magic[0] == 'M' &&
		           magic[1] == 'M' &&
		           magic[2] == 0 &&
		           magic[3] == 42);

		if (II || MM) return true;
		else return false;
	}

	unsigned char* ImageLoadTIF(const std::string aFile, unsigned int* aWidth, unsigned int* aHeight, unsigned int* aBPP)
	{
		COLUMBUS_ASSERT_MESSAGE(aWidth, "ImageLoadTIF(): invalid width")
		COLUMBUS_ASSERT_MESSAGE(aHeight, "ImageLoadTIF(): invalid height")
		COLUMBUS_ASSERT_MESSAGE(aBPP, "ImageLoadTIF(): invalid BPP")

		TIFF* tif = TIFFOpen(aFile.c_str(), "r");
		if (tif == nullptr) return nullptr;

		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int bpp = 0;

		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &bpp);

		*aWidth = width;
		*aHeight = height;
		*aBPP = bpp;

		uint32_t* buffer = (uint32_t*)malloc(width * height * sizeof(uint32_t));
		TIFFReadRGBAImage(tif, width, height, buffer, 0);

		uint8_t* data = (uint8_t*)malloc(width * height * bpp);
		for (size_t i = 0; i < width * height * bpp; i += bpp)
		{
			data[i + 0] = TIFFGetR(*buffer);
			data[i + 1] = TIFFGetG(*buffer);
			data[i + 2] = TIFFGetB(*buffer);
			if (bpp == 4)
				data[i + 3] = TIFFGetA(*buffer);

			buffer++;
		}

		buffer -= width * height;
		free(buffer);

		TIFFClose(tif);

		return data;
	}
	
	bool ImageSaveTIF(const std::string aFile, const unsigned int aWidth, const unsigned int aHeight, const unsigned int aBPP, const unsigned char* aData)
	{

	}

}

