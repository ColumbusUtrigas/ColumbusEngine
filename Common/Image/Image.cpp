#include <Common/Image/Image.h>
#include <Math/MathUtil.h>
#include <Math/Vector4.h>
#include <cstdlib>
#include <cstring>

// third party compression
#include "Lib/bc7enc/rgbcx.h"
#include "Lib/bc7enc/bc7enc.h"
#include "Lib/compressonator/cmp_compressonatorlib/compressonator.h"

namespace Columbus
{

	static TextureFormatInfo TextureFormats[] =
	{
		//                  Name                Format                           BPP   BlockSize  Channels Alpha Compressed  Depth  Stencil
		TextureFormatInfo { "R8",               TextureFormat::R8,               8,    0,         1,       0,    0,          0,     0 },
		TextureFormatInfo { "RG8",              TextureFormat::RG8,              16,   0,         2,       0,    0,          0,     0 },
		TextureFormatInfo { "RGB8",             TextureFormat::RGB8,             24,   0,         3,       0,    0,          0,     0 },
		TextureFormatInfo { "RGBA8",            TextureFormat::RGBA8,            32,   0,         4,       1,    0,          0,     0 },
		TextureFormatInfo { "BGRA8",            TextureFormat::BGRA8,            32,   0,         4,       1,    0,          0,     0 },
		TextureFormatInfo { "R16",              TextureFormat::R16,              16,   0,         1,       0,    0,          0,     0 },
		TextureFormatInfo { "RG16",             TextureFormat::RG16,             32,   0,         2,       0,    0,          0,     0 },
		TextureFormatInfo { "RGB16",            TextureFormat::RGB16,            48,   0,         3,       0,    0,          0,     0 },
		TextureFormatInfo { "RGBA16",           TextureFormat::RGBA16,           64,   0,         4,       1,    0,          0,     0 },
		TextureFormatInfo { "R16F",             TextureFormat::R16F,             16,   0,         1,       0,    0,          0,     0 },
		TextureFormatInfo { "RG16F",            TextureFormat::RG16F,            32,   0,         2,       0,    0,          0,     0 },
		TextureFormatInfo { "RGB16F",           TextureFormat::RGB16F,           48,   0,         3,       1,    0,          0,     0 },
		TextureFormatInfo { "RGBA16F",          TextureFormat::RGBA16F,          64,   0,         4,       1,    0,          0,     0 },
		TextureFormatInfo { "R32F",             TextureFormat::R32F,             32,   0,         1,       0,    0,          0,     0 },
		TextureFormatInfo { "RG32F",            TextureFormat::RG32F,            64,   0,         2,       0,    0,          0,     0 },
		TextureFormatInfo { "RGB32F",           TextureFormat::RGB32F,           96,   0,         3,       0,    0,          0,     0 },
		TextureFormatInfo { "RGBA32F",          TextureFormat::RGBA32F,          128,  0,         4,       1,    0,          0,     0 },
		TextureFormatInfo { "R8UInt",           TextureFormat::R8UInt,           8,    0,         1,       0,    0,          0,     0 },
		TextureFormatInfo { "R32UInt",          TextureFormat::R32UInt,          32,   0,         1,       0,    0,          0,     0 },
		TextureFormatInfo { "R8SRGB",           TextureFormat::R8SRGB,           8,    0,         1,       0,    0,          0,     0 },
		TextureFormatInfo { "RG8SRGB",          TextureFormat::RG8SRGB,          16,   0,         2,       0,    0,          0,     0 },
		TextureFormatInfo { "RGB8SRGB",         TextureFormat::RGB8SRGB,         24,   0,         3,       0,    0,          0,     0 },
		TextureFormatInfo { "RGBA8SRGB",        TextureFormat::RGBA8SRGB,        32,   0,         4,       1,    0,          0,     0 },
		TextureFormatInfo { "BGRA8SRGB",        TextureFormat::BGRA8SRGB,        32,   0,         4,       1,    0,          0,     0 },
		TextureFormatInfo { "R11G11B10F",       TextureFormat::R11G11B10F,       32,   0,         3,       0,    0,          0,     0 },
		TextureFormatInfo { "DXT1",             TextureFormat::DXT1,             4,   64,         4,       0,    1,          0,     0 },
		TextureFormatInfo { "DXT3",             TextureFormat::DXT3,             8,   64,         4,       1,    1,          0,     0 },
		TextureFormatInfo { "DXT5",             TextureFormat::DXT5,             8,  128,         4,       1,    1,          0,     0 },
		TextureFormatInfo { "BC6H",             TextureFormat::BC6H,             8,  128,         4,       0,    1,          0,     0 },
		TextureFormatInfo { "BC7",              TextureFormat::BC7,              8,  128,         4,       1,    1,          0,     0 },
		TextureFormatInfo { "Depth16",          TextureFormat::Depth16,          16,   0,         1,       0,    0,          1,     0 },
		TextureFormatInfo { "Depth24",          TextureFormat::Depth24,          24,   0,         1,       0,    0,          1,     0 },
		TextureFormatInfo { "Depth24Stencil8",  TextureFormat::Depth24Stencil8,  32,   0,         2,       0,    0,          1,     1 },
		TextureFormatInfo { "Depth32F",         TextureFormat::Depth32F,         32,   0,         1,       0,    0,          1,     0 },
		TextureFormatInfo { "Depth32FStencil8", TextureFormat::Depth32FStencil8, 40,   0,         2,       0,    0,          1,     1 },
		TextureFormatInfo { "Unknown",          TextureFormat::Unknown,          0,    0,         0,       0,    0,          0,     0 },
	};

	static CMP_FORMAT CompressonatorFormatMap[] =
	{
		CMP_FORMAT_R_8,       // R8
		CMP_FORMAT_RG_8,      // RG8
		CMP_FORMAT_RGB_888,   // RGB8
		CMP_FORMAT_RGBA_8888, // RGBA8
		CMP_FORMAT_BGRA_8888, // BGRA8

		CMP_FORMAT_R_16,    // R16
		CMP_FORMAT_RG_16,   // RG16
		CMP_FORMAT_Unknown, // RGB16
		CMP_FORMAT_RGBA_16, // RGBA16

		CMP_FORMAT_R_16F,    // R16F
		CMP_FORMAT_RG_16F,   // RG16F
		CMP_FORMAT_Unknown,  // RGB16F
		CMP_FORMAT_RGBA_16F, // RGBA16F

		CMP_FORMAT_R_32F,     // R32F
		CMP_FORMAT_RG_32F,    // RG32F
		CMP_FORMAT_RGB_32F,  // RGB32F
		CMP_FORMAT_RGBA_32F, // RGBA32F

		CMP_FORMAT_R_8,     // R8UInt
		CMP_FORMAT_Unknown, // R32UInt

		CMP_FORMAT_Unknown, // R8SRGB,
		CMP_FORMAT_Unknown, // RG8SRGB,
		CMP_FORMAT_Unknown, // RGB8SRGB,
		CMP_FORMAT_Unknown, // RGBA8SRGB,
		CMP_FORMAT_Unknown, // BGRA8SRGB,

		CMP_FORMAT_Unknown, // R11G11B10F

		CMP_FORMAT_DXT1, // DXT1
		CMP_FORMAT_DXT3, // DXT3
		CMP_FORMAT_DXT5, // DXT5
		CMP_FORMAT_BC6H, // BC6H
		CMP_FORMAT_BC7,  // BC7

		CMP_FORMAT_Unknown, // Depth16
		CMP_FORMAT_Unknown, // Depth24
		CMP_FORMAT_Unknown, // Depth24Stencil8
		CMP_FORMAT_Unknown, // Depth32F
		CMP_FORMAT_Unknown, // Depth32FStencil8
		CMP_FORMAT_Unknown, // Unknown
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

		bool ImageLoadFromStream(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutDepth, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData)
		{
			ImageFormat Format = ImageGetFileFormatFromStream(Stream);
			Stream.SeekSet(0); // rewind to start

			OutDepth = 1;

			switch (Format)
			{
			case ImageFormat::DDS: return ImageLoadFromStreamDDS(Stream, OutWidth, OutHeight, OutDepth, OutMips, OutFormat, OutType, OutData);
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

			return Size * (Depth > 0 ? Depth : 1);
		}

		u64 ImageCalcByteSize(u32 Width, u32 Height, u32 Depth, u32 Mips, u32 Layers, TextureFormat Format)
		{
			return ImageCalcByteSize(Width, Height, Depth, Mips, Format) * (u64)Layers;
		}

		u64 ImageCalcByteSize(u32 Width, u32 Height, u32 Depth, u32 Mips, TextureFormat Format, ImageType Type)
		{
			return ImageCalcByteSize(Width, Height, Depth, Mips, Type == ImageType::ImageCube ? 6 : 1, Format);
		}

		u64 ImageCalcByteSizeForMip(u32 Width, u32 Height, u32 Depth, u32 Mip, TextureFormat Format)
		{
			TextureFormatInfo FormatInfo = TextureFormatGetInfo(Format);

			if (FormatInfo.HasCompression)
			{
				return (((Width >> Mip) + 3) / 4) * (((Height >> Mip) + 3) / 4) * Math::Max(1u, Depth >> Mip) * (FormatInfo.CompressedBlockSizeBits / 8);
			}
			else
			{
				return ((Width >> Mip) * (Height >> Mip) * Math::Max(1u, Depth >> Mip) * FormatInfo.BitsPerPixel) / 8;
			}

			return 0;
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

	void Image::AllocImage(u32 W, u32 H, u32 D, u32 Mips, TextureFormat InFormat, ImageType InType)
	{
		FreeData();
		Width = W;
		Height = H;
		Depth = D;
		MipMaps = Mips;
		Format = InFormat;
		Type = InType;
		Exist = true;

		Data = new u8[GetFullSize()];
	}

	ImageMip Image::GetMip(u32 Mip, u32 Layer) const
	{
		ImageMip Result;
		Result.Width  = Math::Max(1u, Width >> Mip);
		Result.Height = Math::Max(1u, Height >> Mip);
		Result.Depth  = Math::Max(1u, Depth >> Mip);
		Result.Format = Format;
		Result.Data = Data + GetOffset(Layer, Mip);

		return Result;
	}

	bool ImageMip::ReadPixelRGBA32F(float Pixel[4], u32 X, u32 Y, u32 Z) const
	{
		X = Math::Clamp(X, 0u, Width  - 1);
		Y = Math::Clamp(Y, 0u, Height - 1);
		Z = Math::Clamp(Z, 0u, Depth  - 1);

		u64 PixelOffset = (TextureFormatGetInfo(Format).BitsPerPixel / 8) * (X + Y * Width + Z * Width * Height);
		u8* PixelPtr = Data + PixelOffset;

		switch (Format)
		{
		case TextureFormat::R8:
			Pixel[0] = PixelPtr[0] / 255.f;
			Pixel[1] = 0.0f;
			Pixel[2] = 0.0f;
			Pixel[3] = 1.0f; // alpha 1
			break;
		case TextureFormat::RG8:
			Pixel[0] = PixelPtr[0] / 255.f;
			Pixel[1] = PixelPtr[1] / 255.f;
			Pixel[2] = 0.0f;
			Pixel[3] = 1.0f; // alpha 1
			break;
		case TextureFormat::RGB8:
			Pixel[0] = PixelPtr[0] / 255.f;
			Pixel[1] = PixelPtr[1] / 255.f;
			Pixel[2] = PixelPtr[2] / 255.f;
			Pixel[3] = 1.0f; // alpha 1
			break;
		case TextureFormat::RGBA8:
			Pixel[0] = PixelPtr[0] / 255.f;
			Pixel[1] = PixelPtr[1] / 255.f;
			Pixel[2] = PixelPtr[2] / 255.f;
			Pixel[3] = PixelPtr[3] / 255.f;
			break;
		case TextureFormat::RGB32F:
		{
			float* PixelPtrF32 = (float*)PixelPtr;
			*Pixel++ = *PixelPtrF32++;
			*Pixel++ = *PixelPtrF32++;
			*Pixel++ = *PixelPtrF32++;
			*Pixel++ = 1.0f; // alpha 1
			break;
		}
		case TextureFormat::RGBA32F:
		{
			float* PixelPtrF32 = (float*)PixelPtr;
			*Pixel++ = *PixelPtrF32++;
			*Pixel++ = *PixelPtrF32++;
			*Pixel++ = *PixelPtrF32++;
			*Pixel++ = *PixelPtrF32++;
			break;
		}
		default:
			// unimplemented
			DEBUGBREAK();
			return false;
			break;
		}

		return true;
	}

	bool ImageMip::WritePixelRGBA32F(float Pixel[4], u32 X, u32 Y, u32 Z) const
	{
		X = Math::Clamp(X, 0u, Width - 1);
		Y = Math::Clamp(Y, 0u, Height - 1);
		Z = Math::Clamp(Z, 0u, Depth - 1);

		u64 PixelOffset = (TextureFormatGetInfo(Format).BitsPerPixel / 8) * (X + Y * Width + Z * Width * Height);
		u8* PixelPtr = Data +PixelOffset;

		switch (Format)
		{
		case TextureFormat::R8:
			PixelPtr[0] = (char)(Pixel[0] * 255.f);
			break;
		case TextureFormat::RG8:
			PixelPtr[0] = (char)(Pixel[0] * 255.f);
			PixelPtr[1] = (char)(Pixel[1] * 255.f);
			break;
		case TextureFormat::RGB8:
			PixelPtr[0] = (char)(Pixel[0] * 255.f);
			PixelPtr[1] = (char)(Pixel[1] * 255.f);
			PixelPtr[2] = (char)(Pixel[2] * 255.f);
			break;
		case TextureFormat::RGBA8:
			PixelPtr[0] = (char)(Pixel[0] * 255.f);
			PixelPtr[1] = (char)(Pixel[1] * 255.f);
			PixelPtr[2] = (char)(Pixel[2] * 255.f);
			PixelPtr[3] = (char)(Pixel[3] * 255.f);
			break;
		case TextureFormat::RGB32F:
		{
			float* PixelPtrF32 = (float*)PixelPtr;
			*PixelPtrF32++ = *Pixel++;
			*PixelPtrF32++ = *Pixel++;
			*PixelPtrF32++ = *Pixel++;
			break;
		}
		case TextureFormat::RGBA32F:
		{
			float* PixelPtrF32 = (float*)PixelPtr;
			*PixelPtrF32++ = *Pixel++;
			*PixelPtrF32++ = *Pixel++;
			*PixelPtrF32++ = *Pixel++;
			*PixelPtrF32++ = *Pixel++;
			break;
		}
		default:
			// unimplemented
			DEBUGBREAK();
			return false;
			break;
		}

		return true;
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

		ImageUtils::ImageLoadFromStream(Stream, Width, Height, Depth, MipMaps, Format, Type, Data);

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
		return ImageUtils::ImageCalcByteSizeForMip(Width, Height, Depth, Level, Format);
	}

	u64 Image::GetTotalPixelCount() const
	{
		TextureFormatInfo FormatInfo = TextureFormatGetInfo(Format);
		return (GetFullSize() * 8) / FormatInfo.BitsPerPixel;
	}

	u64 Image::GetFullSize() const
	{
		return ImageUtils::ImageCalcByteSize(Width, Height, Depth, MipMaps, Format, Type);
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

	namespace ImageMips
	{

		void GenerateImageMips(const Image& Img, Image& DstImage, MipGenerationParams Params)
		{
			TextureFormatInfo FormatInfo = TextureFormatGetInfo(Img.Format);
			if (FormatInfo.HasCompression)
			{
				Log::Error("[GenerateImageMips] Cannot generate mips for compressed image, format is %s", FormatInfo.FriendlyName);
				DEBUGBREAK();
				return;
			}

			u32 NumMips = (u32)log2(Math::Max(Img.Height, Img.Width)) + 1;

			DstImage.AllocImage(Img.Width, Img.Height, Img.Depth, NumMips, Img.Format, Img.Type);

			u64 FirstMipSize = ImageUtils::ImageCalcByteSizeForMip(Img.Width, Img.Height, Img.Depth, 0, Img.Format);
			u32 NumLayers = Img.Type == ImageType::ImageCube ? 6 : 1;

			for (u32 layer = 0; layer < NumLayers; layer++)
			{
				u64 SrcLayerOffset = Img.GetOffset(layer, 0);
				u64 DstLayerOffset = DstImage.GetOffset(layer, 0);

				// copy first mip of the layer
				memcpy(DstImage.Data + DstLayerOffset, Img.Data + SrcLayerOffset, FirstMipSize);

				for (u32 mip = 0; mip < (NumMips - 1); mip++)
				{
					ImageMip CurrentMip = DstImage.GetMip(mip, layer);
					ImageMip NextMip = DstImage.GetMip(mip + 1, layer);

					int SampleOffsetsX[] = { 0, 1, 1, 0, 0, 1, 1, 0 };
					int SampleOffsetsY[] = { 0, 0, 1, 1, 0, 0, 1, 1 };
					int SampleOffsetsZ[] = { 0, 0, 0, 0, 1, 1, 1, 1 };
					int NumSamples = Img.Type == ImageType::Image3D ? 8 : 4;

					// compute next mip
					for (u32 z = 0; z < NextMip.Depth; z++)
					{
						for (u32 y = 0; y < NextMip.Height; y++)
						{
							for (u32 x = 0; x < NextMip.Width; x++)
							{
								Vector4 Pixels[8];

								for (int i = 0; i < NumSamples; i++)
								{
									CurrentMip.ReadPixelRGBA32F((float*)&Pixels[i], x * 2 + SampleOffsetsX[i], y * 2 + SampleOffsetsY[i], z * 2 + SampleOffsetsZ[i]);
								}

								Vector4 AveragePixel(0);
								for (int i = 0; i < NumSamples; i++)
								{
									AveragePixel += Pixels[i];
								}
								AveragePixel /= (float)NumSamples;

								NextMip.WritePixelRGBA32F((float*)&AveragePixel, x, y, z);
							}
						}
					}
				}
			}
		}

	} // namespace ImageMips

	namespace ImageCompression
	{

		void InitImageCompression()
		{
			rgbcx::init();
			bc7enc_compress_block_init();
			CMP_InitializeBCLibrary();
		}

		bool CompressImageBcEnc(const Image& Img, Image& DstImage, CompressionParams Params)
		{
			TextureFormatInfo SourceFormatInfo = TextureFormatGetInfo(Img.Format);
			TextureFormatInfo TargetFormatInfo = TextureFormatGetInfo(Params.Format);

			// sanity checks
			{
				if (SourceFormatInfo.HasCompression)
				{
					Log::Error("[ImageCompression] Source image is already compressed with format %s", SourceFormatInfo.FriendlyName);
					DEBUGBREAK();
					return false;
				}

				if (Img.Type != ImageType::Image2D)
				{
					Log::Error("[ImageCompression] Source Image type is not supported, only 2D");
					DEBUGBREAK();
					return false;
				}

				switch (Img.Format)
				{
				case TextureFormat::R8: break;
				case TextureFormat::RGB8: break;
				case TextureFormat::RGBA8: break;
				default:
					Log::Error("[ImageCompression] Source Image format is not supported: %s", SourceFormatInfo.FriendlyName);
					DEBUGBREAK();
					return false;
					break;
				}

				switch (Params.Format)
				{
				case TextureFormat::DXT1:
				case TextureFormat::DXT5:
				case TextureFormat::BC7: break;
				default:
					Log::Error("[ImageCompression] Compression Target Image format is not supported: %s", TargetFormatInfo.FriendlyName);
					DEBUGBREAK();
					return false;
					break;
				}
			}

			// data initialisation
			DstImage.FreeData();
			DstImage.Format = Params.Format;
			DstImage.Type = Img.Type;
			DstImage.Exist = true;
			DstImage.Width = Img.Width;
			DstImage.Height = Img.Height;
			DstImage.Depth = Img.Depth;
			DstImage.MipMaps = 1; // TODO: generate mips as well

			u64 SrcDataSize = ImageUtils::ImageCalcByteSize(Img.Width, Img.Height, Img.Depth, Img.MipMaps, Img.Format);
			u64 DstDataSize = ImageUtils::ImageCalcByteSize(DstImage.Width, DstImage.Height, DstImage.Depth, DstImage.MipMaps, DstImage.Format);
			DstImage.Data = new u8[DstDataSize];

			u32 BlocksX = (DstImage.Width + 3) / 4;
			u32 BlocksY = (DstImage.Height + 3) / 4;

			TextureFormatInfo FormatInfo = TextureFormatGetInfo(DstImage.Format);

			// TODO: Image::ReadPixelRGBA8 with account for formats would be useful
			// TODO: it should also read mips
			const auto ReadPixelRGBA8 = [&Img, &SourceFormatInfo](u32 x, u32 y, u8 outPixel[4])
			{
				u32 PixelOffset = y * Img.Width + x;

				switch (SourceFormatInfo.Format)
				{
				case TextureFormat::R8:
					outPixel[0] = Img.Data[PixelOffset];
					outPixel[1] = 0;
					outPixel[2] = 0;
					outPixel[3] = 255;
					break;
				case TextureFormat::RGB8:
					outPixel[0] = Img.Data[PixelOffset * 3 + 0];
					outPixel[1] = Img.Data[PixelOffset * 3 + 1];
					outPixel[2] = Img.Data[PixelOffset * 3 + 2];
					outPixel[3] = 255;
					break;
				case TextureFormat::RGBA8:
					outPixel[0] = Img.Data[PixelOffset * 4 + 0];
					outPixel[1] = Img.Data[PixelOffset * 4 + 1];
					outPixel[2] = Img.Data[PixelOffset * 4 + 2];
					outPixel[3] = Img.Data[PixelOffset * 4 + 3];
					break;
				default: break;
				}
			};

			u64 DstDataOffset = 0;
			u32 BlockSizeBytes = FormatInfo.CompressedBlockSizeBits / 8;

			u32 Uberlevel = 0; // wtf is this?

			bc7enc_compress_block_params pack_params;
			bc7enc_compress_block_params_init(&pack_params);
			if (!Params.PerceptualMetric)
				bc7enc_compress_block_params_init_linear_weights(&pack_params);
			pack_params.m_max_partitions_mode = BC7ENC_MAX_PARTITIONS1;
			pack_params.m_uber_level = Uberlevel;

			for (u32 y = 0; y < BlocksY; y++)
			{
				for (u32 x = 0; x < BlocksX; x++)
				{
					u8 BlockPixels[4][4][4]; // 4x4 rgba8

					// filling the block
					for (int i = 0; i < 4; i++)
					{
						for (int j = 0; j < 4; j++)
						{
							ReadPixelRGBA8(x * 4 + i, y * 4 + j, BlockPixels[j][i]);
						}
					}

					u64 Block[2];

					switch (Params.Format)
					{
					case TextureFormat::DXT1:
						rgbcx::encode_bc1(Uberlevel, &Block, (u8*)BlockPixels, false, false);
						break;
					case TextureFormat::DXT5:
						rgbcx::encode_bc3(Uberlevel, &Block, (u8*)BlockPixels);
						break;
					case TextureFormat::BC7:
						bc7enc_compress_block(&Block, BlockPixels, &pack_params);
						break;
					default: break;
					}

					// write block
					memcpy(DstImage.Data + DstDataOffset, &Block, BlockSizeBytes);
					DstDataOffset += BlockSizeBytes;
				}
			}

			return true;
		}

		bool CompressImageCompressonator(const Image& InImg, Image& DstImage, CompressionParams Params)
		{
			Image TranscodedImage;

			const Image* ImgPtr = &InImg;

			// transcoding
			// TODO: new unified API for transcoding
			{
				TranscodedImage.Width = InImg.Width;
				TranscodedImage.Height = InImg.Height;
				TranscodedImage.Depth = InImg.Depth;
				TranscodedImage.MipMaps = InImg.MipMaps;
				TranscodedImage.Type = InImg.Type;

				if (InImg.Format == TextureFormat::RGB16)
				{
					TranscodedImage.Format = TextureFormat::RGBA16;
					TranscodedImage.Data = new u8[TranscodedImage.GetFullSize()];

					u16* SrcData = (u16*)InImg.Data;
					u16* DstData = (u16*)TranscodedImage.Data;

					u64 PixelCount = InImg.GetTotalPixelCount();
					for (u64 i = 0; i < PixelCount; i++)
					{
						*DstData++ = *SrcData++;
						*DstData++ = *SrcData++;
						*DstData++ = *SrcData++;
						*DstData++ = 0xffff; // alpha
					}

					ImgPtr = &TranscodedImage;
				}

				if (InImg.Format == TextureFormat::RGB16F)
				{
					TranscodedImage.Format = TextureFormat::RGBA16F;
					TranscodedImage.Data = new u8[TranscodedImage.GetFullSize()];

					u16* SrcData = (u16*)InImg.Data;
					u16* DstData = (u16*)TranscodedImage.Data;

					u64 PixelCount = InImg.GetTotalPixelCount();
					for (u64 i = 0; i < PixelCount; i++)
					{
						*DstData++ = *SrcData++;
						*DstData++ = *SrcData++;
						*DstData++ = *SrcData++;
						*DstData++ = 0x3c00; // alpha 1
					}

					ImgPtr = &TranscodedImage;
				}

				if (InImg.Format == TextureFormat::RGB32F)
				{
					TranscodedImage.Format = TextureFormat::RGBA32F;
					TranscodedImage.Data = new u8[TranscodedImage.GetFullSize()];

					float* SrcData = (float*)InImg.Data;
					float* DstData = (float*)TranscodedImage.Data;

					u64 PixelCount = InImg.GetTotalPixelCount();
					for (u64 i = 0; i < PixelCount; i++)
					{
						*DstData++ = *SrcData++;
						*DstData++ = *SrcData++;
						*DstData++ = *SrcData++;
						*DstData++ = 1.0f; // alpha
					}

					ImgPtr = &TranscodedImage;
				}
			}

			const Image& Img = *ImgPtr;

			TextureFormatInfo SourceFormatInfo = TextureFormatGetInfo(Img.Format);
			TextureFormatInfo TargetFormatInfo = TextureFormatGetInfo(Params.Format);

			CMP_FORMAT SourceFormatCmp = CompressonatorFormatMap[(int)Img.Format];
			CMP_FORMAT TargetFormatCmp = CompressonatorFormatMap[(int)Params.Format];

			// sanity checks
			{
				if (SourceFormatCmp == CMP_FORMAT_Unknown)
				{
					Log::Error("[ImageCompression] Source format is not supported: %s", SourceFormatInfo.FriendlyName);
					DEBUGBREAK();
					return false;
				}

				if (TargetFormatCmp == CMP_FORMAT_Unknown)
				{
					Log::Error("[ImageCompression] Target format is not supported: %s", TargetFormatInfo.FriendlyName);
					DEBUGBREAK();
					return false;
				}
			}

			// TODO: new api for image creation
			// data initialisation
			DstImage.FreeData();
			DstImage.Format = Params.Format;
			DstImage.Type = Img.Type;
			DstImage.Exist = true;
			DstImage.Width = Img.Width;
			DstImage.Height = Img.Height;
			DstImage.Depth = Math::Max(1u, Img.Depth);
			DstImage.MipMaps = Img.MipMaps;

			u32 LayerCount = Img.Type == ImageType::ImageCube ? 6 : 1;

			u64 SrcDataSize = ImageUtils::ImageCalcByteSize(Img.Width, Img.Height, Img.Depth, Img.MipMaps, Img.Format) * LayerCount;
			u64 DstDataSize = ImageUtils::ImageCalcByteSize(DstImage.Width, DstImage.Height, DstImage.Depth, DstImage.MipMaps, DstImage.Format) * LayerCount;
			DstImage.Data = new u8[DstDataSize];

			CMP_CompressOptions Options{ 0 };
			Options.dwSize = sizeof(Options);
			Options.fquality = 0.1f;
			Options.dwnumThreads = 0;  // Uses auto, else set number of threads from 1..127 max
			// BC1-3 refinement
			Options.bUseRefinementSteps = 1;
			Options.nRefinementSteps = 1;

			// run through mips, run through 3d textures, run through cubemap faces and compress them as if they are textures
			for (u32 layer = 0; layer < LayerCount; layer++)
			{
				for (u32 mip = 0; mip < Img.MipMaps; mip++)
				{
					u32 MipWidth  = Math::Max<u32>(1, Img.Width  / (int)pow(2, mip));
					u32 MipHeight = Math::Max<u32>(1, Img.Height / (int)pow(2, mip));
					u32 MipDepth  = Math::Max<u32>(1, Img.Depth  / (int)pow(2, mip));

					u64 SrcDepthOffset = 0;
					u64 DstDepthOffset = 0;

					for (int depth = 0; depth < Math::Max<int>(1, Img.Depth); depth++)
					{
						u64 SrcMipDepthSliceSize = ImageUtils::ImageCalcByteSize(MipWidth, MipHeight, 1, 1, Img.Format);
						u64 DstMipDepthSliceSize = ImageUtils::ImageCalcByteSize(MipWidth, MipHeight, 1, 1, DstImage.Format);

						CMP_Texture SrcTexture{ 0 };
						SrcTexture.dwSize = sizeof(CMP_Texture);
						SrcTexture.dwWidth = MipWidth;
						SrcTexture.dwHeight = MipHeight;
						SrcTexture.dwPitch = SourceFormatInfo.HasCompression ? 0 : ((MipWidth * SourceFormatInfo.BitsPerPixel) / 8);
						SrcTexture.format = SourceFormatCmp;
						SrcTexture.dwDataSize = (u32)SrcMipDepthSliceSize;
						SrcTexture.pData = Img.Data + Img.GetOffset(layer, mip) + SrcDepthOffset;

						CMP_Texture DstTexture = SrcTexture;
						DstTexture.dwPitch = TargetFormatInfo.HasCompression ? 0 : ((MipWidth * TargetFormatInfo.BitsPerPixel) / 8);
						DstTexture.format = TargetFormatCmp;
						DstTexture.dwDataSize = (u32)DstMipDepthSliceSize;
						DstTexture.pData = DstImage.Data + DstImage.GetOffset(layer, mip) + DstDepthOffset;

						SrcDepthOffset += SrcMipDepthSliceSize;
						DstDepthOffset += DstMipDepthSliceSize;

						CMP_ERROR Err = CMP_ConvertTexture(&SrcTexture, &DstTexture, &Options, [](float Progress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2)
						{
							return false; // true to abort compression
						});

						if (Err != CMP_OK)
						{
							return false;
						}
					}
				}
			}

			return true;
		}

		bool CompressImage(const Image& Img, Image& DstImage, CompressionParams Params)
		{
			// TODO: choose bc7enc for more efficient bc7 encoding, make it multithreaded, support mips, volumes, cubemaps
			return CompressImageCompressonator(Img, DstImage, Params);
		}

		bool DecompressImage(const Image& Img, Image& DstImage)
		{
			TextureFormatInfo FormatInfo = TextureFormatGetInfo(Img.Format);

			if (!FormatInfo.HasCompression)
			{
				// just copy
				DstImage = Img;
				return false;
			}

			TextureFormat SelectedFormat = TextureFormat::Unknown;

			switch (Img.Format)
			{
			case TextureFormat::DXT1:
			case TextureFormat::DXT3:
			case TextureFormat::DXT5:
				SelectedFormat = TextureFormat::RGBA8;
				break;
			case TextureFormat::BC6H:
			case TextureFormat::BC7:
				SelectedFormat = TextureFormat::RGBA32F;
				break;
			default:
				Log::Error("[DecompressImage] Format isn't implemented %s", FormatInfo.FriendlyName);
				DEBUGBREAK();
				break;
			}

			CompressionParams Params;
			Params.Format = SelectedFormat;

			return CompressImageCompressonator(Img, DstImage, Params);
		}

	} // namespace ImageCompression

}
