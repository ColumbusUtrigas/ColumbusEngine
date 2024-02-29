#include <Common/Image/Image.h>
#include <cstdlib>
#include <cstring>

namespace Columbus
{

	static TextureFormatInfo TextureFormats[] =
	{
		//                  Name                Format                           BPP   BlockSize  Channels  Compressed  Depth  Stencil
		TextureFormatInfo { "R8",               TextureFormat::R8,               8,    0,         1,        0,          0,     0 },
		TextureFormatInfo { "RG8",              TextureFormat::RG8,              16,   0,         2,        0,          0,     0 },
		TextureFormatInfo { "RGB8",             TextureFormat::RGB8,             24,   0,         3,        0,          0,     0 },
		TextureFormatInfo { "RGBA8",            TextureFormat::RGBA8,            32,   0,         4,        0,          0,     0 },
		TextureFormatInfo { "BGRA8",            TextureFormat::BGRA8,            32,   0,         4,        0,          0,     0 },
		TextureFormatInfo { "R16",              TextureFormat::R16,              16,   0,         1,        0,          0,     0 },
		TextureFormatInfo { "RG16",             TextureFormat::RG16,             32,   0,         2,        0,          0,     0 },
		TextureFormatInfo { "RGB16",            TextureFormat::RGB16,            48,   0,         3,        0,          0,     0 },
		TextureFormatInfo { "RGBA16",           TextureFormat::RGBA16,           64,   0,         4,        0,          0,     0 },
		TextureFormatInfo { "R16F",             TextureFormat::R16F,             16,   0,         1,        0,          0,     0 },
		TextureFormatInfo { "RG16F",            TextureFormat::RG16F,            32,   0,         2,        0,          0,     0 },
		TextureFormatInfo { "RGB16F",           TextureFormat::RGB16F,           48,   0,         3,        0,          0,     0 },
		TextureFormatInfo { "RGBA16F",          TextureFormat::RGBA16F,          64,   0,         4,        0,          0,     0 },
		TextureFormatInfo { "R32F",             TextureFormat::R32F,             32,   0,         1,        0,          0,     0 },
		TextureFormatInfo { "RG32F",            TextureFormat::RG32F,            64,   0,         2,        0,          0,     0 },
		TextureFormatInfo { "RGB32F",           TextureFormat::RGB32F,           96,   0,         3,        0,          0,     0 },
		TextureFormatInfo { "RGBA32F",          TextureFormat::RGBA32F,          128,  0,         4,        0,          0,     0 },
		TextureFormatInfo { "R8UInt",           TextureFormat::R8UInt,           8,    0,         1,        0,          0,     0 },
		TextureFormatInfo { "R32UInt",          TextureFormat::R32UInt,          32,   0,         1,        0,          0,     0 },
		TextureFormatInfo { "R8SRGB",           TextureFormat::R8SRGB,           8,    0,         1,        0,          0,     0 },
		TextureFormatInfo { "RG8SRGB",          TextureFormat::RG8SRGB,          16,   0,         2,        0,          0,     0 },
		TextureFormatInfo { "RGB8SRGB",         TextureFormat::RGB8SRGB,         24,   0,         3,        0,          0,     0 },
		TextureFormatInfo { "RGBA8SRGB",        TextureFormat::RGBA8SRGB,        32,   0,         4,        0,          0,     0 },
		TextureFormatInfo { "BGRA8SRGB",        TextureFormat::BGRA8SRGB,        32,   0,         4,        0,          0,     0 },
		TextureFormatInfo { "R11G11B10F",       TextureFormat::R11G11B10F,       32,   0,         3,        0,          0,     0 },
		TextureFormatInfo { "DXT1",             TextureFormat::DXT1,             8,   64,         4,        1,          0,     0 },
		TextureFormatInfo { "DXT3",             TextureFormat::DXT3,             8,   64,         4,        1,          0,     0 },
		TextureFormatInfo { "DXT5",             TextureFormat::DXT5,             8,  128,         4,        1,          0,     0 },
		TextureFormatInfo { "BC6H",             TextureFormat::BC6H,             8,  128,         4,        1,          0,     0 },
		TextureFormatInfo { "BC7",              TextureFormat::BC7,              8,  128,         4,        1,          0,     0 },
		TextureFormatInfo { "Depth16",          TextureFormat::Depth16,          16,   0,         1,        0,          1,     0 },
		TextureFormatInfo { "Depth24",          TextureFormat::Depth24,          24,   0,         1,        0,          1,     0 },
		TextureFormatInfo { "Depth24Stencil8",  TextureFormat::Depth24Stencil8,  32,   0,         2,        0,          1,     1 },
		TextureFormatInfo { "Depth32F",         TextureFormat::Depth32F,         32,   0,         1,        0,          1,     0 },
		TextureFormatInfo { "Depth32FStencil8", TextureFormat::Depth32FStencil8, 40,   0,         2,        0,          1,     1 },
		TextureFormatInfo { "Unknown",          TextureFormat::Unknown,          0,    0,         0,        0,          0,     0 },
	};

	const TextureFormatInfo& TextureFormatGetInfo(TextureFormat Format)
	{
		return TextureFormats[(u32)Format];
	}

	namespace ImageUtils
	{
		ImageFormat ImageGetFileFormatFromStream(DataStream& Stream)
		{
			if (!Stream.IsValid())
				return ImageFormat::Unknown;

			if (ImageCheckFormatFromStreamDDS(Stream)) return ImageFormat::DDS;
			if (ImageCheckFormatFromStreamPNG(Stream)) return ImageFormat::PNG;
			if (ImageCheckFormatFromStreamJPG(Stream)) return ImageFormat::JPG;
			if (ImageCheckFormatFromStreamEXR(Stream)) return ImageFormat::EXR;
			if (ImageCheckFormatFromStreamHDR(Stream)) return ImageFormat::HDR;
			if (ImageCheckFormatFromStreamBMP(Stream)) return ImageFormat::BMP;
			if (ImageCheckFormatFromStreamTIF(Stream)) return ImageFormat::TIF;
			if (ImageCheckFormatFromStreamTGA(Stream)) return ImageFormat::TGA;

			return ImageFormat::Unknown;
		}

		bool ImageLoadFromStream(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData)
		{
			ImageFormat Format = ImageGetFileFormatFromStream(Stream);
			Stream.SeekSet(0); // rewind to start

			switch (Format)
			{
			case ImageFormat::DDS: return ImageLoadFromStreamDDS(Stream, OutWidth, OutHeight, OutMips, OutFormat, OutType, OutData);
			case ImageFormat::PNG: return ImageLoadFromStreamPNG(Stream, OutWidth, OutHeight, OutMips, OutFormat, OutType, OutData);
			case ImageFormat::JPG: return ImageLoadFromStreamJPG(Stream, OutWidth, OutHeight, OutMips, OutFormat, OutType, OutData);
			case ImageFormat::EXR: return ImageLoadFromStreamEXR(Stream, OutWidth, OutHeight, OutMips, OutFormat, OutType, OutData);
			case ImageFormat::HDR: return ImageLoadFromStreamHDR(Stream, OutWidth, OutHeight, OutMips, OutFormat, OutType, OutData);
			case ImageFormat::BMP: return ImageLoadFromStreamBMP(Stream, OutWidth, OutHeight, OutMips, OutFormat, OutType, OutData);
			case ImageFormat::TIF: return ImageLoadFromStreamTIF(Stream, OutWidth, OutHeight, OutMips, OutFormat, OutType, OutData);
			case ImageFormat::TGA: return ImageLoadFromStreamTGA(Stream, OutWidth, OutHeight, OutMips, OutFormat, OutType, OutData);
			default: return false;
			}

			return false;
		}

		u64 ImageCalcByteSize(u32 Width, u32 Height, u32 Depth, u32 Mips, TextureFormat Format)
		{
			u64 Size = 0;

			TextureFormatInfo Info = TextureFormatGetInfo(Format);

			if (Info.HasCompression)
			{
				u32 BlockSize = Info.CompressedBlockSizeBits / 8;
				for (u32 Level = 0; Level < Mips; Level++)
				{
					Size += (((Width >> Level) + 3) / 4) * (((Height >> Level) + 3) / 4) * BlockSize;
				}
			}
			else
			{
				u32 BPP = Info.BitsPerPixel / 8;
				for (u32 Level = 0; Level < Mips; Level++)
				{
					Size += (Width >> Level) * (Height >> Level) * BPP;
				}
			}

			return Size * Depth;
		}

		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		bool ImageBGR2RGB(u8* aData, u64 aSize)
		{
			if (aData == nullptr) return false;

			u8 bgr[3];
			for (u64 i = 0; i < aSize; i += 3)
			{
				bgr[0] = aData[i + 0];
				bgr[1] = aData[i + 1];
				bgr[2] = aData[i + 2];

				aData[i + 0] = bgr[2];
				aData[i + 1] = bgr[1];
				aData[i + 2] = bgr[0];
			}

			return true;
		}

		bool ImageBGRA2RGBA(u8* aData, u64 aSize)
		{
			if (aData == nullptr) return false;

			u8 bgr[3];
			for (u64 i = 0; i < aSize; i += 4)
			{
				bgr[0] = aData[i + 0];
				bgr[1] = aData[i + 1];
				bgr[2] = aData[i + 2];

				aData[i + 0] = bgr[2];
				aData[i + 1] = bgr[1];
				aData[i + 2] = bgr[0];
				//aData[i + 3] = aData[i + 3];
			}

			return true;
		}

		bool ImageABGR2RGBA(u8* aData, u64 aSize)
		{
			if (aData == nullptr) return false;

			u8 abgr[4];
			for (u64 i = 0; i < aSize; i += 4)
			{
				abgr[0] = aData[i + 0];
				abgr[1] = aData[i + 1];
				abgr[2] = aData[i + 2];
				abgr[3] = aData[i + 3];

				aData[i + 0] = abgr[3];
				aData[i + 1] = abgr[2];
				aData[i + 2] = abgr[1];
				aData[i + 3] = abgr[0];
			}

			return true;
		}

		bool ImageRGB2BGR(u8* aData, u64 aSize)
		{
			return ImageBGR2RGB(aData, aSize);
		}

		bool ImageRGBA2BGRA(u8* aData, u64 aSize)
		{
			return ImageBGRA2RGBA(aData, aSize);
		}

		bool ImageFlipX(u8* Data, u32 Width, u32 Height, u32 BPP)
		{
			if (Data == nullptr) return false;

			const u32 stride = Width * BPP;
			u8* row;
			u8* pixel = new u8[BPP];

			for (u32 i = 0; i < Height; i++)
			{
				row = &Data[i * stride];

				for (u32 j = 0; j < Width / 2; j++)
				{
					memcpy(pixel, &row[j * BPP], BPP);
					memcpy(&row[j * BPP], &row[(Width - j) * BPP], BPP);
					memcpy(&row[(Width - j) * BPP], pixel, BPP);
				}
			}

			return true;
		}

		bool ImageFlipY(u8* Data, u32 Width, u32 Height, u32 BPP)
		{
			if (Data == nullptr) return false;

			const u32 stride = Width * BPP;
			u8* row = (u8*)malloc(stride);

			for (u32 i = 0; i < Height / 2; i++)
			{
				memcpy(row, &Data[i * stride], stride);
				memcpy(&Data[i * stride], &Data[(Height - i - 1) * stride], stride);
				memcpy(&Data[(Height - i - 1) * stride], row, stride);
			}
			free(row);

			return true;
		}

		bool ImageFlipXY(u8* Data, u32 Width, u32 Height, u32 BPP)
		{
			if (Data == nullptr) return false;

			const u32 size = Width * Height;
			u8* pixel = new u8[BPP];

			for (size_t i = 0; i < size / 2; i++)
			{
				memcpy(pixel, &Data[i * BPP], BPP);
				memcpy(&Data[i * BPP], &Data[(size - i) * BPP], BPP);
				memcpy(&Data[(size - i) * BPP], pixel, BPP);
			}

			delete[] pixel;

			return true;
		}
	}

	/*
	* Load image from file
	* @param const char* InFileName: Name of image file to load
	* @param int Flags: Loading flags
	*/
	bool Image::LoadFromFile(std::string_view InFileName, ImageLoading Flags)
	{
		DataStream Stream = DataStream::CreateFromFile(std::string(InFileName).c_str(), "rb");
		return LoadFromStream(Stream, Flags);
	}

	/*
	* Load image from memory
	* @param const u8* Memory: Memory of size Size where image will be loaded from
	* @param const u64 Size: Size of memory block to be loaded
	* @param int Flags: Loading flags
	*/
	bool Image::LoadFromMemory(const u8* Memory, const u64 Size, ImageLoading Flags)
	{
		DataStream Stream = DataStream::CreateFromMemory((u8*)Memory, Size);
		return LoadFromStream(Stream, Flags);
	}

	bool Image::LoadFromStream(DataStream& Stream, ImageLoading Flags)
	{
		FreeData();

		ImageUtils::ImageLoadFromStream(Stream, Width, Height, MipMaps, Format, Type, Data);

		if (Data == nullptr)
		{
			return false;
		}

		Exist = true;

		u32 BPP = TextureFormatGetInfo(Format).BitsPerPixel / 8;

		switch (Flags)
		{
		case ImageLoading::FlipX:  ImageUtils::ImageFlipX(Data, Width, Height, BPP);  break;
		case ImageLoading::FlipY:  ImageUtils::ImageFlipY(Data, Width, Height, BPP);  break;
		case ImageLoading::FlipXY: ImageUtils::ImageFlipXY(Data, Width, Height, BPP); break;
		case ImageLoading::None: break;
		}

		return true;
	}

	/*
	* Save image to file
	* @param const char* InFileName: Name of image file to save
	* @param Format: Image format
	* @param Quality: Compression level
	*/
	bool Image::SaveToFile(const char* InFileName, ImageFormat InFormat, u32 Quality) const
	{
		if (!IsExist())
		{
			return false;
		}

		switch (InFormat)
		{
		case ImageFormat::BMP: return ImageUtils::ImageSaveToFileBMP(InFileName, Width, Height, Format, Data);          break;
		case ImageFormat::JPG: return ImageUtils::ImageSaveToFileJPG(InFileName, Width, Height, Format, Data, Quality); break;
		case ImageFormat::PNG: return ImageUtils::ImageSaveToFilePNG(InFileName, Width, Height, Format, Data);          break;
		case ImageFormat::TGA: return ImageUtils::ImageSaveToFileTGA(InFileName, Width, Height, Format, Data);          break;
		case ImageFormat::TIF: return ImageUtils::ImageSaveToFileTIF(InFileName, Width, Height, Format, Data);          break;
		default: break;
		}
		
		return false;
	}
	/*
	* Frees image data
	*/
	void Image::FreeData()
	{
		Width = 0;
		Height = 0;
		Size = 0;
		MipMaps = 0;
		Format = TextureFormat::RGBA8;
		Exist = false;

		delete[] Data;
		Data = nullptr;
	}

	u64 Image::GetOffset(u32 Layer, u32 Level) const
	{
		if (Level < MipMaps)
		{
			u64 Offset = 0;
			u64 LayerSize = 0;

			for (u32 i = 0; i < Level; i++)
			{
				Offset += GetSize(i);
			}

			for (u32 i = 0; i < MipMaps; i++)
			{
				LayerSize += GetSize(i);
			}

			return Layer * LayerSize + Offset;
		}

		return 0;
	}

	u64 Image::GetSize(u32 Level) const
	{
		TextureFormatInfo FormatInfo = TextureFormatGetInfo(Format);

		if (FormatInfo.HasCompression)
		{
			return (((Width >> Level) + 3) / 4) * (((Height >> Level) + 3) / 4) * (FormatInfo.CompressedBlockSizeBits / 8);
		}
		else
		{
			return (Width >> Level) * (Height >> Level) * (FormatInfo.BitsPerPixel / 8);
		}

		return 0;
	}

	u8* Image::Get2DData(u32 Level) const
	{
		if (Type == ImageType::Image2D && Data != nullptr)
		{
			return &Data[0] + GetOffset(0, Level);
		}

		return nullptr;
	}

	u8* Image::GetCubeData(u32 Face, u32 Level) const
	{
		if (Type == ImageType::ImageCube && Data != nullptr)
		{
			return &Data[0] + GetOffset(Face, Level);
		}

		return nullptr;
	}
	
	Image::~Image()
	{
		FreeData();
	}
}
