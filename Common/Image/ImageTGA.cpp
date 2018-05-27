#include <Common/Image/Image.h>
#include <Core/Memory.h>
#include <System/File.h>

namespace Columbus
{

#define READPIXEL24(a) \
		blue = *a++;   \
		green = *a++;  \
		red = *a++;

#define READPIXEL32(a) \
		READPIXEL24(a) \
		alpha = *a++;

#define WRITEPIXEL24(a) \
		*a++ = red;     \
		*a++ = green;   \
		*a++ = blue;

#define WRITEPIXEL32(a) \
		WRITEPIXEL24(a) \
		*a++ = alpha;


	typedef struct
	{
		uint8 idlen;          //Image ID Length
		uint8 color_map_type; //Color map type
		uint8 image_type;     //Image Type
							/*
							0  -  No image data included.
							1  -  Uncompressed, color-mapped images.
							2  -  Uncompressed, RGB images.
							3  -  Uncompressed, black and white images.
							9  -  Runlength encoded color-mapped images.
							10  - Runlength encoded RGB images.
							11  - Compressed, black and white images.
							32  - Compressed color-mapped data, using Huffman, Delta, and runlength encoding.
							33  - Compressed color-mapped data, using Huffman, Delta, and runlength encoding.  4-pass quadtree-type process.
							*/
		uint16 color_map_origin;    //Color
		uint16 color_map_length;    //map
		uint8 color_map_entry_size; //specification

		uint16 x_origin; //
		uint16 y_origin; //Image
		uint16 width;    //specification
		uint16 height;   //
		uint8 bits;      // Bit depth. 8, 16, 24 or 32
		uint8 image_descriptor;
	} TGA_HEADER;

	static bool ReadHeader(TGA_HEADER* aHeader, File* aFile)
	{
		if (aHeader == nullptr || aFile == nullptr) return false;

		if (!aFile->ReadUint8(&aHeader->idlen)) return false;
		if (!aFile->ReadUint8(&aHeader->color_map_type)) return false;
		if (!aFile->ReadUint8(&aHeader->image_type)) return false;
		if (!aFile->ReadUint16(&aHeader->color_map_origin)) return false;
		if (!aFile->ReadUint16(&aHeader->color_map_length)) return false;
		if (!aFile->ReadUint8(&aHeader->color_map_entry_size)) return false;
		if (!aFile->ReadUint16(&aHeader->x_origin)) return false;
		if (!aFile->ReadUint16(&aHeader->y_origin)) return false;
		if (!aFile->ReadUint16(&aHeader->width)) return false;
		if (!aFile->ReadUint16(&aHeader->height)) return false;
		if (!aFile->ReadUint8(&aHeader->bits)) return false;
		if (!aFile->ReadUint8(&aHeader->image_descriptor)) return false;

		return true;
	}

	static bool WriteHeader(TGA_HEADER aHeader, File* aFile)
	{
		if (aFile == nullptr) return false;

		if (!aFile->WriteUint8(aHeader.idlen)) return false;
		if (!aFile->WriteUint8(aHeader.color_map_type)) return false;
		if (!aFile->WriteUint8(aHeader.image_type)) return false;
		if (!aFile->WriteUint16(aHeader.color_map_origin)) return false;
		if (!aFile->WriteUint16(aHeader.color_map_length)) return false;
		if (!aFile->WriteUint8(aHeader.color_map_entry_size)) return false;
		if (!aFile->WriteUint16(aHeader.x_origin)) return false;
		if (!aFile->WriteUint16(aHeader.y_origin)) return false;
		if (!aFile->WriteUint16(aHeader.width)) return false;
		if (!aFile->WriteUint16(aHeader.height)) return false;
		if (!aFile->WriteUint8(aHeader.bits)) return false;
		if (!aFile->WriteUint8(aHeader.image_descriptor)) return false;

		return true;
	}

	bool ImageIsTGA(std::string FileName)
	{
		std::string ext = FileName.substr(FileName.size() - 4);

		if (ext == ".tga" || ext == ".vda" ||
		    ext == ".icb" || ext == ".vst") return true;

		return false;
	}

	static void RGBCompressedTGA(uint8* InBuffer, uint8* OutBuffer, size_t Size)
	{
		COLUMBUS_ASSERT_MESSAGE(InBuffer, "TGA RGB compression: invalid input")
		COLUMBUS_ASSERT_MESSAGE(OutBuffer, "TGA RGB compression: invalid output")

		int header;
		int blue, green, red;
		size_t i, j, pixelcount;

		for (i = 0; i < Size; )
		{
			header = *InBuffer++;
			pixelcount = (header & 0x7f) + 1;

			if (header & 0x80)
			{
				READPIXEL24(InBuffer)
				for (j = 0; j < pixelcount; j++)
				{
					WRITEPIXEL24(OutBuffer)
				}
				i += pixelcount;
			} else
			{
				for (j = 0; j < pixelcount; j++)
				{
					READPIXEL24(InBuffer)
					WRITEPIXEL24(OutBuffer)
				}
				i += pixelcount;
			}
		}
	}

	static void RGBACompressedTGA(uint8* InBuffer, uint8* OutBuffer, size_t Size)
	{
		COLUMBUS_ASSERT_MESSAGE(InBuffer, "TGA RGB compression: invalid input")
		COLUMBUS_ASSERT_MESSAGE(OutBuffer, "TGA RGB compression: invalid output")

		int header;
		int blue, green, red, alpha;
		int pix;
		size_t i, j, pixelcount;

		for (i = 0; i < Size; )
		{
			header = *InBuffer++;
			pixelcount = (header & 0x7f) + 1;
			if (header & 0x80)
			{
				READPIXEL32(InBuffer);
				pix = red | (green << 8) | (blue << 16) | (alpha << 24);

				for (j = 0; j < pixelcount; j++)
				{
					memcpy(OutBuffer, &pix, 4);
					OutBuffer += 4;
				}

				i += pixelcount;
			}
			else
			{
				for (j = 0; j < pixelcount; j++)
				{
					READPIXEL32(InBuffer)
					WRITEPIXEL32(OutBuffer)
				}
				i += pixelcount;
			}
		}
	}

	uint8* ImageLoadTGA(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint64& OutSize, TextureFormat& OutFormat)
	{
		File file(FileName, "rb");
		if (!file.IsOpened()) return nullptr;

		TGA_HEADER tga;

		if (!ReadHeader(&tga, &file)) return nullptr;

		size_t dSize = file.GetSize() - sizeof(TGA_HEADER);
		size_t size = tga.width * tga.height * tga.bits / 8;

		uint8* buffer = new uint8[dSize];
		file.Read(buffer, dSize, 1);

		uint8* data = nullptr;

		switch (tga.image_type)
		{
		case 2:
			//Uncompressed RGB
			data = new uint8[size];

			if (tga.bits == 24)
			{
				std::copy(&buffer[0], &buffer[size], &data[0]);

				for (uint64 i = 0; i < size; i += 3)
				{
					std::swap(data[i + 0], data[i + 2]);
				}
			}
			else if (tga.bits == 32)
			{
				std::copy(&buffer[0], &buffer[size], &data[0]);

				for (uint64 i = 0; i < size; i += 4)
				{
					std::swap(data[i + 0], data[i + 2]);
				}
			}

			break;
		case 10:
			//Compressed RGB
			data = new uint8[size];

			if (tga.bits == 24)
			{
				RGBCompressedTGA(buffer, data, tga.width * tga.height);
			} else
			{
				RGBACompressedTGA(buffer, data, tga.width * tga.height);
			}
			break;
		}

		if (tga.x_origin != 0) ImageFlipX(buffer, tga.width, tga.height, tga.bits / 8);
		if (tga.y_origin != 0) ImageFlipY(buffer, tga.width, tga.height, tga.bits / 8);

		file.Close();

		OutWidth = tga.width;
		OutHeight = tga.height;
		OutSize = tga.width * tga.height * tga.bits / 8;

		switch (tga.bits)
		{
		case 24: OutFormat = TextureFormat::RGB;  break;
		case 32: OutFormat = TextureFormat::RGBA; break;
		};

		delete[] buffer;
		return data;
	}

	bool ImageSaveTGA(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data)
	{
		if (Data == nullptr) return false;

		File file(FileName, "wb");
		if (!file.IsOpened()) return false;

		uint32 BPP = GetBPPFromFormat(Format);

		uint16 width = static_cast<uint16>(Width);
		uint16 height = static_cast<uint16>(Height);
		uint8 bpp = static_cast<uint8>(BPP * 8);
		uint8 descriptor = static_cast<uint8>(8);

		TGA_HEADER tga = { 0, 0, 2, 0, 0, 0, 0, 0, width, height, bpp, descriptor};

		size_t size = Width * Height * BPP;

		uint8* buffer = (uint8*)Memory::Malloc(size);
		Memory::Memcpy(buffer, Data, size);

		switch (tga.bits)
		{
		case 24: ImageRGB2BGR(buffer, size); break;
		case 32: ImageRGBA2BGRA(buffer, size); break;
		};

		WriteHeader(tga, &file);
		file.Write(buffer, size, 1);

		Memory::Free(buffer);
		return true;
	}

#undef READPIXEL24
#undef WRITEPIXEL24
#undef READPIXEL32
#undef WRITEPIXEL32

}






