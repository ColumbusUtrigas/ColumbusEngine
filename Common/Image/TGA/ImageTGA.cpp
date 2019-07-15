#include <Common/Image/Image.h>
#include <Common/Image/TGA/ImageTGA.h>
#include <System/Assert.h>
#include <System/File.h>
#include <algorithm>
#include <utility>
#include <cstring>

namespace Columbus
{

#define READPIXEL8(a) \
		red = *a++;

#define READPIXEL24(a) \
		blue = *a++;   \
		green = *a++;  \
		red = *a++;

#define READPIXEL32(a) \
		READPIXEL24(a) \
		alpha = *a++;

#define WRITEPIXEL8(a) \
		*a++ = red;

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

		if (!aFile->Read(aHeader->idlen)) return false;
		if (!aFile->Read(aHeader->color_map_type)) return false;
		if (!aFile->Read(aHeader->image_type)) return false;
		if (!aFile->Read(aHeader->color_map_origin)) return false;
		if (!aFile->Read(aHeader->color_map_length)) return false;
		if (!aFile->Read(aHeader->color_map_entry_size)) return false;
		if (!aFile->Read(aHeader->x_origin)) return false;
		if (!aFile->Read(aHeader->y_origin)) return false;
		if (!aFile->Read(aHeader->width)) return false;
		if (!aFile->Read(aHeader->height)) return false;
		if (!aFile->Read(aHeader->bits)) return false;
		if (!aFile->Read(aHeader->image_descriptor)) return false;

		return true;
	}

	static bool WriteHeader(TGA_HEADER aHeader, File* aFile)
	{
		if (aFile == nullptr) return false;

		if (!aFile->Write(aHeader.idlen)) return false;
		if (!aFile->Write(aHeader.color_map_type)) return false;
		if (!aFile->Write(aHeader.image_type)) return false;
		if (!aFile->Write(aHeader.color_map_origin)) return false;
		if (!aFile->Write(aHeader.color_map_length)) return false;
		if (!aFile->Write(aHeader.color_map_entry_size)) return false;
		if (!aFile->Write(aHeader.x_origin)) return false;
		if (!aFile->Write(aHeader.y_origin)) return false;
		if (!aFile->Write(aHeader.width)) return false;
		if (!aFile->Write(aHeader.height)) return false;
		if (!aFile->Write(aHeader.bits)) return false;
		if (!aFile->Write(aHeader.image_descriptor)) return false;

		return true;
	}

	template <typename Type>
	static void RGBPalettedTGA(Type* InBuffer, uint8* ColorMap, uint8* OutBuffer, size_t Size)
	{
		COLUMBUS_ASSERT_MESSAGE(InBuffer, "TGA RGB paletted: invalid input");
		COLUMBUS_ASSERT_MESSAGE(ColorMap, "TGA RGB paletted: invalid input");
		COLUMBUS_ASSERT_MESSAGE(OutBuffer, "TGA RGB paletted: invalid output");

		const int PixelSize = 3;
		int Index;
		int red, green, blue;
		size_t i;
		uint8* ColorMapPtr;

		for (i = 0; i < Size; i++)
		{
			Index = InBuffer[i];
			ColorMapPtr = &ColorMap[Index * PixelSize];

			READPIXEL24(ColorMapPtr);
			WRITEPIXEL24(OutBuffer);
		}
	}

	template <typename Type>
	static void RGBAPalettedTGA(Type* InBuffer, uint8* ColorMap, uint8* OutBuffer, size_t Size)
	{
		COLUMBUS_ASSERT_MESSAGE(InBuffer, "TGA RGBA paletted: invalid input");
		COLUMBUS_ASSERT_MESSAGE(ColorMap, "TGA RGBA paletted: invalid input");
		COLUMBUS_ASSERT_MESSAGE(OutBuffer, "TGA RGBA paletted: invalid output");

		const int PixelSize = 4;
		int Index;
		int red, green, blue, alpha;
		size_t i;
		uint8* ColorMapPtr;

		for (i = 0; i < Size; i++)
		{
			Index = InBuffer[i];
			ColorMapPtr = &ColorMap[Index * PixelSize];

			READPIXEL32(ColorMapPtr);
			WRITEPIXEL32(OutBuffer);
		}
	}

	static void RCompressedTGA(uint8* InBuffer, uint8* OutBuffer, size_t Size)
	{
		COLUMBUS_ASSERT_MESSAGE(InBuffer, "TGA R compressed: invalid input");
		COLUMBUS_ASSERT_MESSAGE(OutBuffer, "TGA R compressed: invalid output");

		int header;
		int red;
		size_t i, j, pixelcount;

		for (i = 0; i < Size; )
		{
			header = *InBuffer++;
			pixelcount = (header & 0x7F) + 1;

			if (header & 0x80)
			{
				READPIXEL8(InBuffer);
				for (j = 0; j < pixelcount; j++)
				{
					WRITEPIXEL8(OutBuffer);
				}
				i += pixelcount;
			} else
			{
				for (j = 0; j < pixelcount; j++)
				{
					READPIXEL8(InBuffer);
					WRITEPIXEL8(OutBuffer);
				}
				i += pixelcount;
			}
		}
	}

	static void RGBCompressedTGA(uint8* InBuffer, uint8* OutBuffer, size_t Size)
	{
		COLUMBUS_ASSERT_MESSAGE(InBuffer, "TGA RGB compressed: invalid input");
		COLUMBUS_ASSERT_MESSAGE(OutBuffer, "TGA RGB compressed: invalid output");

		int header;
		int blue, green, red;
		size_t i, j, pixelcount;

		for (i = 0; i < Size; )
		{
			header = *InBuffer++;
			pixelcount = (header & 0x7F) + 1;

			if (header & 0x80)
			{
				READPIXEL24(InBuffer);
				for (j = 0; j < pixelcount; j++)
				{
					WRITEPIXEL24(OutBuffer);
				}
				i += pixelcount;
			} else
			{
				for (j = 0; j < pixelcount; j++)
				{
					READPIXEL24(InBuffer);
					WRITEPIXEL24(OutBuffer);
				}
				i += pixelcount;
			}
		}
	}

	/*static void RGBACompressedTGA(uint8* InBuffer, uint8* OutBuffer, size_t Size)
	{
		COLUMBUS_ASSERT_MESSAGE(InBuffer, "TGA RGBA compressed: invalid input");
		COLUMBUS_ASSERT_MESSAGE(OutBuffer, "TGA RGBA compressed: invalid output");

		int header;
		int blue, green, red, alpha;
		int pix;
		size_t i, j, pixelcount;

		for (i = 0; i < Size; )
		{
			header = *InBuffer++;
			pixelcount = (header & 0x7F) + 1;
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
					READPIXEL32(InBuffer);
					WRITEPIXEL32(OutBuffer);
				}
				i += pixelcount;
			}
		}
	}*/

	static uint8* ImageLoadTGA(const char* FileName, uint32& OutWidth, uint32& OutHeight, uint64& OutSize, TextureFormat& OutFormat)
	{
		File file(FileName, "rb");
		if (!file.IsOpened()) return nullptr;

		TGA_HEADER tga;
		if (!ReadHeader(&tga, &file)) return nullptr;

		uint8* Descriptor = new uint8[tga.image_descriptor];
		file.ReadBytes(Descriptor, tga.image_descriptor);

		size_t ColorMapElementSize = tga.color_map_entry_size / 8;
		size_t ColorMapSize = tga.color_map_length * ColorMapElementSize;
		uint8* ColorMap = new uint8[tga.color_map_type == 1 ? ColorMapSize : 1];
		if (tga.color_map_type == 1)
		{
			file.ReadBytes(ColorMap, ColorMapSize);
		}

		size_t PixelSize = tga.color_map_type == 0 ? (tga.bits / 8) : ColorMapElementSize;
		size_t dSize = file.GetSize() - sizeof(TGA_HEADER) - tga.image_descriptor - ColorMapSize;
		size_t size = tga.width * tga.height * PixelSize;

		uint8* buffer = new uint8[dSize];
		file.Read(buffer, dSize, 1);

		uint8* data = new uint8[size];
		memset(data, 0, size);

		switch (tga.image_type)
		{
			case 0: break; //No image
			case 1: //Uncompressed paletted
			{
				if (tga.bits == 8)
				{
					switch (PixelSize)
					{
						case 3: RGBPalettedTGA((uint8*)buffer, ColorMap, data, tga.width * tga.height);  break;
						case 4: RGBAPalettedTGA((uint8*)buffer, ColorMap, data, tga.width * tga.height); break;
					}
				}
				else if (tga.bits == 16)
				{
					switch (PixelSize)
					{
						case 3: RGBPalettedTGA((uint16*)buffer, ColorMap, data, tga.width * tga.height);  break;
						case 4: RGBAPalettedTGA((uint16*)buffer, ColorMap, data, tga.width * tga.height); break;
					}
				}

				break;
			}

			case 2: //Uncompressed RGB
			{
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
			}
			case 3: //Uncompressed monochrome
			{
				if (tga.bits == 8)
				{
					std::copy(&buffer[0], &buffer[size], &data[0]);
				}

				break;
			}
			case 9: break;//Compressed paletted TODO
			case 10: //Compressed RGB
			{
				if (tga.bits == 24)
				{
					RGBCompressedTGA(buffer, data, tga.width * tga.height);
				} else
				{
					COLUMBUS_ASSERT_MESSAGE(false, "RGBACompressedTGA() didn't implemented");
					// TODO
					//RGBACompressedTGA(buffer, data, tga.width * tga.height);
				}
				break;
			}
			case 11: //Compressed monochrome
			{
				if (tga.bits == 8)
				{
					RCompressedTGA(buffer, data, tga.width * tga.height);
				}

				break;
			}
		}

		if (tga.x_origin != 0) ImageFlipX(data, tga.width, tga.height, PixelSize);
		if (tga.y_origin == 0) ImageFlipY(data, tga.width, tga.height, PixelSize);

		file.Close();

		OutWidth = tga.width;
		OutHeight = tga.height;
		OutSize = tga.width * tga.height * PixelSize;

		switch (PixelSize)
		{
		case 1: OutFormat = TextureFormat::R8;    break;
		case 3: OutFormat = TextureFormat::RGB8;  break;
		case 4: OutFormat = TextureFormat::RGBA8; break;
		};

		delete[] Descriptor;
		delete[] ColorMap;
		delete[] buffer;
		return data;
	}

	bool ImageLoaderTGA::IsTGA(const char* FileName)
	{
		char ext[5] = { '\0' };
		strncpy(ext, &FileName[strlen(FileName) - 4], 4);

		if (strcmp(ext, ".tga") == 0 || strcmp(ext, ".vda") == 0 ||
		    strcmp(ext, ".icb") == 0 || strcmp(ext, ".vst") == 0) return true;

		return false;
	}

	bool ImageLoaderTGA::Load(const char* FileName)
	{
		uint64 Size;
		Data = ImageLoadTGA(FileName, Width, Height, Size, Format);
		ImageType = ImageLoader::Type::Image2D;
		return (Data != nullptr);
	}

	bool ImageSaveTGA(const char* FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data)
	{
		if (Data == nullptr) return false;

		File file(FileName, "wb");
		if (!file.IsOpened()) return false;

		uint32 BPP = GetBPPFromFormat(Format);

		uint16 width = static_cast<uint16>(Width);
		uint16 height = static_cast<uint16>(Height);
		uint8 bpp = static_cast<uint8>(BPP * 8);
		uint8 descriptor = static_cast<uint8>(8);

		TGA_HEADER tga = { 0, 0, 2, 0, 0, 0, 0, 0, width, height, bpp, descriptor };

		size_t size = Width * Height * BPP;

		uint8* buffer = (uint8*)malloc(size);
		memcpy(buffer, Data, size);

		switch (tga.bits)
		{
		case 24: ImageRGB2BGR(buffer, size); break;
		case 32: ImageRGBA2BGRA(buffer, size); break;
		};

		WriteHeader(tga, &file);
		file.Write(buffer, size, 1);

		free(buffer);
		return true;
	}

#undef READPIXEL8
#undef READPIXEL24
#undef READPIXEL32
#undef WRITEPIXEL8
#undef WRITEPIXEL24
#undef WRITEPIXEL32

}


