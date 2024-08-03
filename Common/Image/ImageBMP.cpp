#include <Common/Image/Image.h>
#include <Core/Assert.h>

namespace Columbus::ImageUtils
{

	typedef struct
	{
		uint16 Magic;
		uint32 Size;
		uint32 Unused;
		uint32 Offset;
	} BMP_HEADER;

	typedef struct
	{
		uint32 InfoSize;
		int32 Width;
		int32 Height;
		uint16 Planes;
		uint16 Bits;
		uint32 Compression;
		uint32 DataSize;
		uint32 HRes;
		uint32 VRes;
		uint32 Colors;
		uint32 ImportantColors;
	} BMP_INFO;

	bool ImageCheckFormatFromStreamBMP(DataStream& Stream)
	{
		u16 Magic;
		Stream.Read(Magic);
		Stream.SeekSet(0); // rewind

		return Magic == 0x4D42; // "BM"
	}

	bool ImageLoadFromStreamBMP(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData)
	{
		if (!Stream.IsValid()) return false;

		BMP_HEADER Header;
		BMP_INFO Info;

		Stream.Read(Header.Magic);
		Stream.Read(Header.Size);
		Stream.Read(Header.Unused);
		Stream.Read(Header.Offset);
		Stream.Read(Info);

		int32 Masks[4];
		int Shifts[4];

		if (Info.Compression == 3)
		{
			Stream.Read(Masks);

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					if (0xFF << j * 8 == Masks[i]) Shifts[i] = j * 8;
				}
			}
		}

		switch (Info.Bits)
		{
		case 8:  OutFormat = TextureFormat::R8;    break;
		case 16: OutFormat = TextureFormat::RG8;   break;
		case 24: OutFormat = TextureFormat::RGB8;  break;
		case 32: OutFormat = TextureFormat::RGBA8; break;
		default: return false; break;
		}

		uint64 Size = Info.Width * Info.Height * (Info.Bits/8);

		OutWidth = Info.Width;
		OutHeight = Info.Height;
		OutMips = 1;
		OutData = new uint8[Size];
		OutType = ImageType::Image2D;

		Stream.SeekSet(Header.Offset);
		Stream.ReadBytes(OutData, Size);

		if (Info.Compression == 3)
		{
			for (uint64 i = 0; i < Size; i += Info.Bits / 8)
			{
				uint32 Pixel = *((uint32*)(&OutData[i]));

				for (int j = 0; j < 4; j++)
				{
					OutData[i + j] = (Pixel & Masks[j]) >> Shifts[j];
				}
			}
		}

		return OutData != nullptr;
	}
	
	bool ImageSaveToFileBMP(const char* FileName, u32 Width, u32 Height, TextureFormat Format, u8* Data)
	{
		COLUMBUS_ASSERT_MESSAGE(false, "Not implemented!");
		return false;
	}

}
