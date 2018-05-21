#include <Common/Image/Image.h>
#include <Core/Memory.h>
#include <System/File.h>

#include <SDL.h>

namespace Columbus
{

	typedef struct
	{
		uint8 magic[2]; //Magic Bytes 'B' and 'M'
		uint32 size;    //Size of whole file
		uint32 unused;  //Should be 0
		uint32 offset;  //Offset to bitmap data
	} BMP_HEADER;

	typedef struct
	{
		uint32 infosize;         //Size of info struct (40 bytes)
		int32 width;             //Width of image
		int32 height;            //Height of image
		uint16 planes;           //Should be 1
		uint16 bits;             //Bits per pixel (1, 4, 8, 16, 24, 32)
		uint32 compression;      //0 = none, 1 = 8-bit RLE, 2 = 4-bit RLE
		uint32 size_data;        //Size of pixel data
		uint32 hres;             //Horizontal resolution (pixel per meter)
		uint32 vres;             //Vertical resolution (pixel per meter)
		uint32 colors;           //Number of palette colors
		uint32 important_colors; //Number of important colors;
	} BMP_INFO;

	static bool ReadHeader(BMP_HEADER* aHeader, File* aFile)
	{
		if (aHeader == nullptr || aFile == nullptr) return false;

		if (!aFile->ReadUint8(&aHeader->magic[0])) return false;
		if (!aFile->ReadUint8(&aHeader->magic[1])) return false;
		if (!aFile->ReadUint32(&aHeader->size)) return false;
		if (!aFile->ReadUint32(&aHeader->unused)) return false;
		if (!aFile->ReadUint32(&aHeader->offset)) return false;

		return true;
	}

	static bool WriteHeader(BMP_HEADER aHeader, File* aFile)
	{
		if (aFile == nullptr) return false;

		if (!aFile->WriteUint8(aHeader.magic[0])) return false;
		if (!aFile->WriteUint8(aHeader.magic[1])) return false;
		if (!aFile->WriteUint32(aHeader.size)) return false;
		if (!aFile->WriteUint32(aHeader.unused)) return false;
		if (!aFile->WriteUint32(aHeader.offset)) return false;

		return true;
	}

	static bool ReadInfo(BMP_INFO* aInfo, File* aFile)
	{
		if (aInfo == nullptr || aFile == nullptr) return false;

		if (!aFile->ReadUint32(&aInfo->infosize)) return false;
		if (!aFile->ReadInt32(&aInfo->width)) return false;
		if (!aFile->ReadInt32(&aInfo->height)) return false;
		if (!aFile->ReadUint16(&aInfo->planes)) return false;
		if (!aFile->ReadUint16(&aInfo->bits)) return false;
		if (!aFile->ReadUint32(&aInfo->compression)) return false;
		if (!aFile->ReadUint32(&aInfo->size_data)) return false;
		if (!aFile->ReadUint32(&aInfo->hres)) return false;
		if (!aFile->ReadUint32(&aInfo->vres)) return false;
		if (!aFile->ReadUint32(&aInfo->colors)) return false;
		if (!aFile->ReadUint32(&aInfo->important_colors)) return false;

		uint8_t* empty = (uint8*)Memory::Malloc(68);
		aFile->ReadBytes(empty, 68);
		free(empty);

		return true;
	}

	static bool WriteInfo(BMP_INFO aInfo, File* aFile)
	{
		if (aFile == nullptr) return false;

		if (!aFile->WriteUint32(aInfo.infosize)) return false;
		if (!aFile->WriteInt32(aInfo.width)) return false;
		if (!aFile->WriteInt32(aInfo.height)) return false;
		if (!aFile->WriteUint16(aInfo.planes)) return false;
		if (!aFile->WriteUint16(aInfo.bits)) return false;
		if (!aFile->WriteUint32(aInfo.compression)) return false;
		if (!aFile->WriteUint32(aInfo.size_data)) return false;
		if (!aFile->WriteUint32(aInfo.hres)) return false;
		if (!aFile->WriteUint32(aInfo.vres)) return false;
		if (!aFile->WriteUint32(aInfo.colors)) return false;
		if (!aFile->WriteUint32(aInfo.important_colors)) return false;

		return true;
	}

	bool ImageIsBMP(std::string FileName)
	{
		File BMPImageFile(FileName, "rb");
		if (!BMPImageFile.IsOpened()) return false;

		uint8_t Magic[2];
		BMPImageFile.Read(Magic, sizeof(Magic), 1);
		BMPImageFile.Close();

		if (Memory::Memcmp(Magic, "BM", 2) == 0)
		{
			return true;
		}

		return false;
	}

	uint8* ImageLoadBMP(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint64& OutSize, TextureFormat& OutFormat)
	{
		File BMPImageFile(FileName, "rb");
		if (!BMPImageFile.IsOpened()) return nullptr;
		BMPImageFile.Close();

		SDL_Surface* Surf = SDL_LoadBMP(FileName.c_str());
		OutWidth = Surf->w;
		OutHeight = Surf->h;
		OutSize = OutWidth * OutHeight * Surf->format->BitsPerPixel / 8;

		switch (Surf->format->BitsPerPixel)
		{
		case 24: OutFormat = TextureFormat::RGB;  break;
		case 32: OutFormat = TextureFormat::RGBA; break;
		}

		uint8* Data = new uint8[OutSize];
		Memory::Memcpy(Data, Surf->pixels, OutSize);
		ImageFlipY(Data, OutWidth, OutHeight, Surf->format->BitsPerPixel / 8);

		/*switch (Surf->format->BitsPerPixel)
		{
		case 24: ImageBGR2RGB(Data, OutSize);   break;
		case 32: ImageBGRA2RGBA(Data, OutSize); break;
		}*/

		return Data;

		/*File BMPImageFile(FileName, "rb");
		if (!BMPImageFile.IsOpened()) return nullptr;

		BMP_HEADER header;
		BMP_INFO info;

		if (!ReadHeader(&header, &BMPImageFile)) return nullptr;
		if (!ReadInfo(&info, &BMPImageFile)) return nullptr;

		uint8* data = (uint8*)Memory::Malloc(header.size);
		BMPImageFile.Read(data, header.size, 1);
		BMPImageFile.Close();

		size_t size = info.width * info.height * info.bits / 8;

		switch (info.bits)
		{
		case 24: ImageBGR2RGB(data, size); break;
		case 32: ImageABGR2RGBA(data, size); break;
		};

		OutWidth = info.width;
		OutHeight = info.height;
		OutSize = info.width * info.height * info.bits / 8;

		switch (info.bits)
		{
		case 24: OutFormat = TextureFormat::RGB;  break;
		case 32: OutFormat = TextureFormat::RGBA; break;
		}

		return data;*/
	}

	bool ImageSaveBMP(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data)
	{
		if (Data == nullptr) return false;

		File file(FileName, "wb");

		BMP_HEADER header;
		BMP_INFO info;

		uint32 BPP = GetBPPFromFormat(Format);

		header.magic[0] = 'B';
		header.magic[1] = 'M';
		header.size = Width * Height * BPP + 54;
		header.unused = 0;
		header.offset = 54;

		info.infosize = 40;
		info.width = Width;
		info.height = Height;
		info.planes = 1;
		info.bits = BPP * 8;
		info.compression = 0;
		info.size_data = Width * Height * BPP;
		info.hres = 0;
		info.vres = 0;
		info.colors = 0;
		info.important_colors = 0;

		if (!WriteHeader(header, &file)) return false;
		if (!WriteInfo(info, &file)) return false;

		uint8* buffer = (uint8*)Memory::Malloc(Width * Height * BPP);
		Memory::Memcpy(buffer, Data, Width * Height * BPP);

		size_t size = Width * Height * BPP;

		switch (BPP * 8)
		{
		case 24: ImageRGB2BGR(buffer, size);   break;
		case 32: ImageRGBA2BGRA(buffer, size); break;
		};

		file.Write(buffer, Width * Height * BPP, 1);

		file.Close();
		Memory::Free(buffer);

		return true;
	}

}



