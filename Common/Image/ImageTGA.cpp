/************************************************
*              	   ImageTGA.cpp                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   03.01.2018                  *
*************************************************/
#include <Common/Image/Image.h>
#include <System/File.h>

namespace Columbus
{

	typedef struct
	{
		uint8_t idlen; //Image ID Lenght
		uint8_t color_map_type; //Color map type
		uint8_t image_type; //Image Type
							/*
							0  -  No image data included.
							1  -  Uncompressed, color-mapped images.
							2  -  Uncompressed, RGB images.
							3  -  Uncompressed, black and white images.
							9  -  Runlength encoded color-mapped images.
							10  -  Runlength encoded RGB images.
							11  -  Compressed, black and white images.
							32  -  Compressed color-mapped data, using Huffman, Delta, and runlength encoding.
							33  -  Compressed color-mapped data, using Huffman, Delta, and runlength encoding.  4-pass quadtree-type process.
							*/
		uint16_t color_map_origin;    //Color
		uint16_t color_map_length;    //map
		uint8_t color_map_entry_size; //specification

		uint16_t x_origin; //
		uint16_t y_origin; //Image
		uint16_t width;    //specification
		uint16_t height;   //
		uint8_t bits; // Bit depth. 8, 16, 24 or 32
		uint8_t image_descriptor;
	} TGA_HEADER;

	static bool ReadHeader(TGA_HEADER* aHeader, C_File* aFile)
	{
		if (aHeader == nullptr || aFile == nullptr) return false;

		if (!aFile->readUint8(&aHeader->idlen)) return false;
		if (!aFile->readUint8(&aHeader->color_map_type)) return false;
		if (!aFile->readUint8(&aHeader->image_type)) return false;
		if (!aFile->readUint16(&aHeader->color_map_origin)) return false;
		if (!aFile->readUint16(&aHeader->color_map_length)) return false;
		if (!aFile->readUint8(&aHeader->color_map_entry_size)) return false;
		if (!aFile->readUint16(&aHeader->x_origin)) return false;
		if (!aFile->readUint16(&aHeader->y_origin)) return false;
		if (!aFile->readUint16(&aHeader->width)) return false;
		if (!aFile->readUint16(&aHeader->height)) return false;
		if (!aFile->readUint8(&aHeader->bits)) return false;
		if (!aFile->readUint8(&aHeader->image_descriptor)) return false;

		return true;
	}

	static bool WriteHeader(TGA_HEADER aHeader, C_File* aFile)
	{
		if (aFile == nullptr) return false;

		if (!aFile->writeUint8(&aHeader.idlen)) return false;
		if (!aFile->writeUint8(&aHeader.color_map_type)) return false;
		if (!aFile->writeUint8(&aHeader.image_type)) return false;
		if (!aFile->writeUint16(&aHeader.color_map_origin)) return false;
		if (!aFile->writeUint16(&aHeader.color_map_length)) return false;
		if (!aFile->writeUint8(&aHeader.color_map_entry_size)) return false;
		if (!aFile->writeUint16(&aHeader.x_origin)) return false;
		if (!aFile->writeUint16(&aHeader.y_origin)) return false;
		if (!aFile->writeUint16(&aHeader.width)) return false;
		if (!aFile->writeUint16(&aHeader.height)) return false;
		if (!aFile->writeUint8(&aHeader.bits)) return false;
		if (!aFile->writeUint8(&aHeader.image_descriptor)) return false;

		return true;
	}

	bool ImageIsTGA(std::string aFile)
	{
		std::string ext = aFile.substr(aFile.size() - 4);

		if (ext == ".tga" || ext == ".vda" ||
			ext == ".icb" || ext == ".vst") return true;

		return false;
	}

	unsigned char* ImageLoadTGA(const std::string aFile, unsigned int* aWidth, unsigned int* aHeight, unsigned int* aBPP)
	{
		C_File file(aFile, "rb");
		if (!file.isOpened()) return false;

		TGA_HEADER tga;

		if (!ReadHeader(&tga, &file)) return false;

		size_t size = tga.width * tga.height * tga.bits / 8;

		uint8_t* buffer = (uint8_t*)malloc(size);

		if (tga.image_type == 2)
		{
			file.read(buffer, size, 1);

			switch (tga.bits)
			{
			case 24:
				ImageBGR2RGB(buffer, size);
				break;
			case 32:
				ImageBGRA2RGBA(buffer, size);
				break;
			};
		}

		if (tga.x_origin != 0)
			ImageFlipX(buffer, tga.width, tga.height, tga.bits / 8);

		if (tga.y_origin != 0)
			ImageFlipY(buffer, tga.width, tga.height, tga.bits / 8);

		file.close();

		*aWidth = tga.width;
		*aHeight = tga.height;
		*aBPP = tga.bits / 8;
		return buffer;
	}

	bool ImageSaveTGA(const std::string aFile, const unsigned int aWidth, const unsigned int aHeight, const unsigned int aBPP, const unsigned char* aData)
	{
		if (aData == nullptr) return false;

		C_File file(aFile, "wb");
		if (!file.isOpened()) return false;

		TGA_HEADER tga = { 0, 0, 2, 0, 0, 0, 0, 0, aWidth, aHeight, aBPP * 8, 8 };

		size_t size = aWidth * aHeight * aBPP;

		uint8_t* buffer = (uint8_t*)malloc(size);
		memcpy(buffer, aData, size);

		switch (aBPP * 8)
		{
		case 24:
			ImageRGB2BGR(buffer, size);
			break;
		case 32:
			ImageRGBA2BGRA(buffer, size);
			break;
		};

		WriteHeader(tga, &file);
		file.write(buffer, size, 1);

		free(buffer);
		return true;
	}

}






