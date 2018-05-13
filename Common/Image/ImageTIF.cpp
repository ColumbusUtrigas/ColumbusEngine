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
#include <Core/Memory.h>
#include <System/File.h>
#include <tiff.h>
#include <tiffio.h>

namespace Columbus
{

	bool ImageIsTIF(std::string FileName)
	{
		File file(FileName, "rb");
		if (!file.IsOpened()) return false;

		uint8_t magic[4];
		if (!file.ReadBytes(magic, sizeof(magic))) return false;
		file.Close();

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

	uint8* ImageLoadTIF(std::string FileName, uint32& OutWidth, uint32& OutHeight, TextureFormat& OutFormat)
	{
		TIFF* tif = TIFFOpen(FileName.c_str(), "r");
		if (tif == nullptr) return nullptr;

		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int bpp = 0;

		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &bpp);

		OutWidth = width;
		OutHeight = height;

		switch (bpp)
		{
		case 3: OutFormat = TextureFormat::RGB;  break;
		case 4: OutFormat = TextureFormat::RGBA; break;
		}

		uint32* buffer = (uint32*)Memory::Malloc(width * height * sizeof(uint32));
		TIFFReadRGBAImage(tif, width, height, buffer, 0);

		uint8* data = (uint8*)Memory::Malloc(width * height * bpp);
		for (size_t i = 0; i < width * height * bpp; i += bpp)
		{
			data[i + 0] = TIFFGetR(*buffer);
			data[i + 1] = TIFFGetG(*buffer);
			data[i + 2] = TIFFGetB(*buffer);
			if (bpp == 4) data[i + 3] = TIFFGetA(*buffer);

			buffer++;
		}

		buffer -= width * height;
		Memory::Free(buffer);

		TIFFClose(tif);

		return data;
	}
	
	bool ImageSaveTIF(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data)
	{
		if (Data == nullptr) return false;

		TIFF* tif = TIFFOpen(FileName.c_str(), "w");
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

		uint8* row = (uint8*)Memory::Malloc(stride);
		uint32 i;

		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, stride));

		for (i = 0; i < Height; i++)
		{
			Memory::Memcpy(row, &Data[(Height - i - 1) * stride], stride);
			if (TIFFWriteScanline(tif, row, i, 0) < 0) break;
		}

		TIFFClose(tif);
		if (row != nullptr)
		{
			Memory::Free(row);
		}

		return true;
	}

}


