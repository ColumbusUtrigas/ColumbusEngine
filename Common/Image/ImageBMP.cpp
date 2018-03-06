/************************************************
*              	   ImageBMP.cpp                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   02.01.2018                  *
*************************************************/
#include <Common/Image/Image.h>
#include <System/File.h>

namespace Columbus
{

	typedef struct
	{
		uint8_t magic[2]; //Magic Bytes 'B' and 'M'
		uint32_t size;    //Size of whole file
		uint32_t unused;  //Should be 0
		uint32_t offset;  //Offset to bitmap data
	} BMP_HEADER;

	typedef struct
	{
		uint32_t infosize;         //Size of info struct (40 bytes)
		int32_t width;             //Width of image
		int32_t height;            //Height of image
		uint16_t planes;           //Should be 1
		uint16_t bits;             //Bits per pixel (1, 4, 8, 16, 24, 32)
		uint32_t compression;      //0 = none, 1 = 8-bit RLE, 2 = 4-bit RLE
		uint32_t size_data;        //Size of pixel data
		uint32_t hres;             //Horizontal resolution (pixel per meter)
		uint32_t vres;             //Vertical resolution (pixel per meter)
		uint32_t colors;           //Number of palette colors
		uint32_t important_colors; //Number of important colors;
	} BMP_INFO;

	static bool ReadHeader(BMP_HEADER* aHeader, C_File* aFile)
	{
		if (aHeader == nullptr || aFile == nullptr) return false;

		if (!aFile->readUint8(&aHeader->magic[0])) return false;
		if (!aFile->readUint8(&aHeader->magic[1])) return false;
		if (!aFile->readUint32(&aHeader->size)) return false;
		if (!aFile->readUint32(&aHeader->unused)) return false;
		if (!aFile->readUint32(&aHeader->offset)) return false;

		return true;
	}

	static bool WriteHeader(BMP_HEADER aHeader, C_File* aFile)
	{
		if (aFile == nullptr) return false;

		if (!aFile->writeUint8(&aHeader.magic[0])) return false;
		if (!aFile->writeUint8(&aHeader.magic[1])) return false;
		if (!aFile->writeUint32(&aHeader.size)) return false;
		if (!aFile->writeUint32(&aHeader.unused)) return false;
		if (!aFile->writeUint32(&aHeader.offset)) return false;

		return true;
	}

	static bool ReadInfo(BMP_INFO* aInfo, C_File* aFile)
	{
		if (aInfo == nullptr || aFile == nullptr) return false;

		if (!aFile->readUint32(&aInfo->infosize)) return false;
		if (!aFile->readInt32(&aInfo->width)) return false;
		if (!aFile->readInt32(&aInfo->height)) return false;
		if (!aFile->readUint16(&aInfo->planes)) return false;
		if (!aFile->readUint16(&aInfo->bits)) return false;
		if (!aFile->readUint32(&aInfo->compression)) return false;
		if (!aFile->readUint32(&aInfo->size_data)) return false;
		if (!aFile->readUint32(&aInfo->hres)) return false;
		if (!aFile->readUint32(&aInfo->vres)) return false;
		if (!aFile->readUint32(&aInfo->colors)) return false;
		if (!aFile->readUint32(&aInfo->important_colors)) return false;

		uint8_t* empty = (uint8_t*)malloc(68);
		aFile->readBytes(empty, 68);
		free(empty);

		return true;
	}

	static bool WriteInfo(BMP_INFO aInfo, C_File* aFile)
	{
		if (aFile == nullptr) return false;

		if (!aFile->writeUint32(&aInfo.infosize)) return false;
		if (!aFile->writeInt32(&aInfo.width)) return false;
		if (!aFile->writeInt32(&aInfo.height)) return false;
		if (!aFile->writeUint16(&aInfo.planes)) return false;
		if (!aFile->writeUint16(&aInfo.bits)) return false;
		if (!aFile->writeUint32(&aInfo.compression)) return false;
		if (!aFile->writeUint32(&aInfo.size_data)) return false;
		if (!aFile->writeUint32(&aInfo.hres)) return false;
		if (!aFile->writeUint32(&aInfo.vres)) return false;
		if (!aFile->writeUint32(&aInfo.colors)) return false;
		if (!aFile->writeUint32(&aInfo.important_colors)) return false;

		return true;
	}

	bool ImageIsBMP(std::string aFile)
	{
		C_File file(aFile, "rb");
		if (!file.isOpened()) return false;

		uint8_t magic[2];
		file.read(magic, sizeof(magic), 1);
		file.close();

		if (magic[0] == 'B' && magic[1] == 'M') return true;
		return false;
	}

	unsigned char* ImageLoadBMP(const std::string aFile, unsigned int& aWidth, unsigned int& aHeight, unsigned int& aBPP)
	{
		C_File file(aFile, "rb");
		if (!file.isOpened()) return nullptr;

		BMP_HEADER header;
		BMP_INFO info;

		if (!ReadHeader(&header, &file)) return nullptr;
		if (!ReadInfo(&info, &file)) return nullptr;

		uint8_t* data = (uint8_t*)malloc(header.size - 66);
		file.read(data, header.size - 66, 1);
		file.close();

		size_t size = info.width * info.height * info.bits / 8;

		switch (info.bits)
		{
		case 24: ImageBGR2RGB(data, size); break;
		case 32: ImageABGR2RGBA(data, size); break;
		};

		aWidth = info.width;
		aHeight = info.height;
		aBPP = info.bits / 8;

		return data;
	}

	bool ImageSaveBMP(const std::string aFile, const unsigned int aWidth, const unsigned int aHeight, const unsigned int aBPP, const unsigned char* aData)
	{
		if (aData == nullptr) return false;

		C_File file(aFile, "wb");

		BMP_HEADER header;
		BMP_INFO info;

		header.magic[0] = 'B';
		header.magic[1] = 'M';
		header.size = aWidth * aHeight * aBPP + 54;
		header.unused = 0;
		header.offset = 54;

		info.infosize = 40;
		info.width = aWidth;
		info.height = aHeight;
		info.planes = 1;
		info.bits = aBPP * 8;
		info.compression = 0;
		info.size_data = aWidth * aHeight * aBPP;
		info.hres = 0;
		info.vres = 0;
		info.colors = 0;
		info.important_colors = 0;

		if (!WriteHeader(header, &file)) return false;
		if (!WriteInfo(info, &file)) return false;

		uint8_t* buffer = (uint8_t*)malloc(aWidth * aHeight * aBPP);
		memcpy(buffer, aData, aWidth * aHeight * aBPP);

		size_t size = aWidth * aHeight * aBPP;

		switch (aBPP * 8)
		{
		case 24:
			ImageRGB2BGR(buffer, size);
			break;
		case 32:
			ImageRGBA2BGRA(buffer, size);
			break;
		};

		file.write(buffer, aWidth * aHeight * aBPP, 1);

		file.close();
		free(buffer);

		return true;
	}

}



