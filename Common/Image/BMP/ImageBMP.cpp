#include <Common/Image/BMP/ImageBMP.h>
#include <Core/Assert.h>
#include <System/File.h>

namespace Columbus
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

	bool ImageLoaderBMP::IsBMP(const char* FileName)
	{
		File BMPImageFile(FileName, "rb");
		if (!BMPImageFile.IsOpened()) return false;

		uint16 Magic;
		BMPImageFile.Read(Magic);
		BMPImageFile.Close();

		return Magic == 0x4D42;
	}

	bool ImageLoaderBMP::Load(const char* FileName)
	{
		File BMPImageFile(FileName, "rb");
		if (!BMPImageFile.IsOpened()) return false;

		BMP_HEADER Header;
		BMP_INFO Info;

		BMPImageFile.Read(Header.Magic);
		BMPImageFile.Read(Header.Size);
		BMPImageFile.Read(Header.Unused);
		BMPImageFile.Read(Header.Offset);
		BMPImageFile.Read(Info);

		int32 Masks[4];
		int Shifts[4];

		if (Info.Compression == 3)
		{
			BMPImageFile.Read(Masks);

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
		case 8:  Format = TextureFormat::R8;    break;
		case 16: Format = TextureFormat::RG8;   break;
		case 24: Format = TextureFormat::RGB8;  break;
		case 32: Format = TextureFormat::RGBA8; break;
		default: return false; break;
		}

		uint64 Size = Info.Width * Info.Height * GetBPPFromFormat(Format);

		Width = Info.Width;
		Height = Info.Height;
		Mipmaps = 1;
		Data = new uint8[Size];
		ImageType = ImageLoader::Type::Image2D;

		BMPImageFile.SeekSet(Header.Offset);
		BMPImageFile.ReadBytes(Data, Size);
		BMPImageFile.Close();

		if (Info.Compression == 3)
		{
			for (uint64 i = 0; i < Size; i += Info.Bits / 8)
			{
				uint32 Pixel = *((uint32*)(&Data[i]));

				for (int j = 0; j < 4; j++)
				{
					Data[i + j] = (Pixel & Masks[j]) >> Shifts[j];
				}
			}
		}

		return Data != nullptr;
	}
	
	//bool ImageSaveBMP(const char* FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data)
	bool ImageSaveBMP(const char*, uint32, uint32, TextureFormat, uint8*)
	{
		COLUMBUS_ASSERT_MESSAGE(false, "Not implemented!");
		return false;
	}

}



