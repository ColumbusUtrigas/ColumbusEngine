#include <Common/Image/Image.h>
#include <Common/Image/BMP/ImageBMP.h>
#include <Common/Image/DDS/ImageDDS.h>
#include <Common/Image/EXR/ImageEXR.h>
#include <Common/Image/HDR/ImageHDR.h>
#include <Common/Image/JPG/ImageJPG.h>
#include <Common/Image/PNG/ImagePNG.h>
#include <Common/Image/TGA/ImageTGA.h>
#include <Common/Image/TIF/ImageTIF.h>
#include <cstdlib>
#include <cstring>

namespace Columbus
{

	uint32 GetBPPFromFormat(TextureFormat Format)
	{
		switch (Format)
		{
			case TextureFormat::R8:      return 1;  break;
			case TextureFormat::RG8:     return 2;  break;
			case TextureFormat::RGB8:    return 3;  break;
			case TextureFormat::RGBA8:   return 4;  break;

			case TextureFormat::R16:    
			case TextureFormat::R16F:    return 2;  break;
			case TextureFormat::RG16:    
			case TextureFormat::RG16F:   return 4;  break;
			case TextureFormat::RGB16:
			case TextureFormat::RGB16F:  return 6;  break;
			case TextureFormat::RGBA16:
			case TextureFormat::RGBA16F: return 8;  break;

			case TextureFormat::R32F:    return 4;  break;
			case TextureFormat::RG32F:   return 8;  break;
			case TextureFormat::RGB32F:  return 12; break;
			case TextureFormat::RGBA32F: return 16; break;

			case TextureFormat::R11G11B10F: return 4; break;

			case TextureFormat::Unknown: return 0;  break;

			default: break;
		}

		return 0;
	}

	uint32 GetBlockSizeFromFormat(TextureFormat Format)
	{
		switch (Format)
		{
		case TextureFormat::DXT1: return 8;  break;
		case TextureFormat::DXT3:
		case TextureFormat::DXT5: return 16; break;
		default: break;
		}

		return 0;
	}

	ImageFormat ImageGetFormat(const char* FileName)
	{
		if (ImageLoaderBMP::IsBMP(FileName)) return ImageFormat::BMP;
		if (ImageLoaderDDS::IsDDS(FileName)) return ImageFormat::DDS;
		if (ImageLoaderEXR::IsEXR(FileName)) return ImageFormat::EXR;
		if (ImageLoaderHDR::IsHDR(FileName)) return ImageFormat::HDR;
		if (ImageLoaderPNG::IsPNG(FileName)) return ImageFormat::PNG;
		if (ImageLoaderTIF::IsTIF(FileName)) return ImageFormat::TIF;
		if (ImageLoaderJPG::IsJPG(FileName)) return ImageFormat::JPG;
		if (ImageLoaderTGA::IsTGA(FileName)) return ImageFormat::TGA;

		return ImageFormat::Unknown;
	}

	uint64 ImageGetSize(uint32 Width, uint32 Height, uint32 Depth, uint32 Mips, TextureFormat Format)
	{
		uint64 Size = 0;

		if (ImageIsCompressedFormat(Format))
		{
			uint32 BlockSize = GetBlockSizeFromFormat(Format);
			for (int Level = 0; Level < Mips; Level++)
			{
				Size += (((Width >> Level) + 3) / 4) * (((Height >> Level) + 3) / 4) * BlockSize;
			}
		}
		else
		{
			uint32 BPP = GetBPPFromFormat(Format);
			for (int Level = 0; Level < Mips; Level++)
			{
				Size += (Width >> Level) * (Height >> Level) * BPP;
			}
		}

		return Size * Depth;
	}

	size_t ImageGetNumChannelsFromFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::R8:
		case TextureFormat::R16:
		case TextureFormat::R16F:
		case TextureFormat::R32F: return 1;

		case TextureFormat::RG8:
		case TextureFormat::RG16:
		case TextureFormat::RG16F:
		case TextureFormat::RG32F: return 2;

		case TextureFormat::RGB8:
		case TextureFormat::RGB16:
		case TextureFormat::RGB16F:
		case TextureFormat::RGB32F:
		case TextureFormat::R11G11B10F: return 3;

		case TextureFormat::RGBA8:
		case TextureFormat::RGBA16:
		case TextureFormat::RGBA16F:
		case TextureFormat::RGBA32F:
		case TextureFormat::DXT1:
		case TextureFormat::DXT3:
		case TextureFormat::DXT5: return 4;
		}

		return 0;
	}

	const char* TextureFormatToString(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::R8:      return "R8";  break;
		case TextureFormat::RG8:     return "RG8";  break;
		case TextureFormat::RGB8:    return "RGB8";  break;
		case TextureFormat::RGBA8:   return "RGBA8";  break;

		case TextureFormat::R16:     return "R16"; break;
		case TextureFormat::R16F:    return "R16F";  break;
		case TextureFormat::RG16:    return "RG16"; break;
		case TextureFormat::RG16F:   return "RG16F";  break;
		case TextureFormat::RGB16:   return "RGB16"; break;
		case TextureFormat::RGB16F:  return "RGB16F";  break;
		case TextureFormat::RGBA16:  return "RGBA16"; break;
		case TextureFormat::RGBA16F: return "RGBA16F";  break;

		case TextureFormat::R32F:    return "R32F";  break;
		case TextureFormat::RG32F:   return "RG32F";  break;
		case TextureFormat::RGB32F:  return "RGB32F"; break;
		case TextureFormat::RGBA32F: return "RGBA32F"; break;

		case TextureFormat::R11G11B10F: return "R11G11B10F"; break;

		case TextureFormat::DXT1: return "DXT1"; break;
		case TextureFormat::DXT3: return "DXT3"; break;
		case TextureFormat::DXT5: return "DXT5"; break;

		case TextureFormat::Depth: return "Depth"; break;
		case TextureFormat::Depth16: return "Depth16"; break;
		case TextureFormat::Depth24: return "Depth24"; break;
		case TextureFormat::Depth24Stencil8: return "Depth24Stencil8"; break;
		case TextureFormat::Depth32F: return "Depth32F"; break;
		case TextureFormat::Depth32FStencil8: return "Depth32FStencil8"; break;

		case TextureFormat::Unknown: return "Unknwon";  break;

		default: break;
		}

		return "Unknown";
	}

	bool ImageIsRawFormat(TextureFormat Format)
	{
		return (Format == TextureFormat::R8 ||
		        Format == TextureFormat::RG8 ||
		        Format == TextureFormat::RGB8 ||
		        Format == TextureFormat::RGBA8);
	}

	bool ImageIsUnsignedShortFormat(TextureFormat Format)
	{
		return (Format == TextureFormat::R16 ||
		        Format == TextureFormat::RG16 ||
		        Format == TextureFormat::RGB16 ||
		        Format == TextureFormat::RGBA16);
	}

	bool ImageIsHalfFormat(TextureFormat Format)
	{
		return (Format == TextureFormat::R16F ||
		        Format == TextureFormat::RG16F ||
		        Format == TextureFormat::RGB16F ||
		        Format == TextureFormat::RGBA16F);
	}

	bool ImageIsFloatFormat(TextureFormat Format)
	{
		return (Format == TextureFormat::R32F ||
		        Format == TextureFormat::RG32F ||
		        Format == TextureFormat::RGB32F ||
		        Format == TextureFormat::RGBA32F);
	}

	bool ImageIsCompressedFormat(TextureFormat Format)
	{
		return (Format == TextureFormat::DXT1 ||
		        Format == TextureFormat::DXT3 ||
		        Format == TextureFormat::DXT5);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool ImageBGR2RGB(uint8* aData, uint64 aSize)
	{
		if (aData == nullptr) return false;

		uint8 bgr[3];
		for (uint64 i = 0; i < aSize; i += 3)
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
	
	bool ImageBGRA2RGBA(uint8* aData, uint64 aSize)
	{
		if (aData == nullptr) return false;

		uint8 bgr[3];
		for (uint64 i = 0; i < aSize; i += 4)
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
	
	bool ImageABGR2RGBA(uint8* aData, uint64 aSize)
	{
		if (aData == nullptr) return false;

		uint8 abgr[4];
		for (uint64 i = 0; i < aSize; i += 4)
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
	
	bool ImageRGB2BGR(uint8* aData, uint64 aSize)
	{
		return ImageBGR2RGB(aData, aSize);
	}
	
	bool ImageRGBA2BGRA(uint8* aData, uint64 aSize)
	{
		return ImageBGRA2RGBA(aData, aSize);
	}
	
	bool ImageFlipX(uint8* Data, uint32 Width, uint32 Height, uint32 BPP)
	{
		if (Data == nullptr) return false;

		const uint32 stride = Width * BPP;
		uint8* row;
		uint8* pixel = new uint8[BPP];

		for (uint32 i = 0; i < Height; i++)
		{
			row = &Data[i * stride];

			for (uint32 j = 0; j < Width / 2; j++)
			{
				memcpy(pixel, &row[j * BPP], BPP);
				memcpy(&row[j * BPP], &row[(Width - j) * BPP], BPP);
				memcpy(&row[(Width - j) * BPP], pixel, BPP);
			}
		}

		return true;
	}
	
	bool ImageFlipY(uint8* Data, uint32 Width, uint32 Height, uint32 BPP)
	{
		if (Data == nullptr) return false;

		const uint32 stride = Width * BPP;
		uint8* row = (uint8_t*)malloc(stride);

		for (uint32 i = 0; i < Height / 2; i++)
		{
			memcpy(row, &Data[i * stride], stride);
			memcpy(&Data[i * stride], &Data[(Height - i - 1) * stride], stride);
			memcpy(&Data[(Height - i - 1) * stride], row, stride);
		}
		free(row);

		return true;
	}
	
	bool ImageFlipXY(uint8* Data, uint32 Width, uint32 Height, uint32 BPP)
	{
		if (Data == nullptr) return false;

		const uint32 size = Width * Height;
		uint8* pixel = new uint8[BPP];

		for (size_t i = 0; i < size / 2; i++)
		{
			memcpy(pixel, &Data[i * BPP], BPP);
			memcpy(&Data[i * BPP], &Data[(size - i) * BPP], BPP);
			memcpy(&Data[(size - i) * BPP], pixel, BPP);
		}

		free(pixel);

		return true;
	}
	/*
	* Image class
	*/
	Image::Image() {}
	/*
	* Load image from file
	* @param const char* InFileName: Name of image file to load
	* @param int Flags: Loading flags
	*/
	bool Image::Load(std::string_view InFileName, ImageLoading Flags)
	{
		FreeData();

		ImageLoader* Loader = nullptr;

		switch (ImageGetFormat(InFileName.data()))
		{
			case ImageFormat::BMP: Loader = new ImageLoaderBMP(); break;
			case ImageFormat::DDS: Loader = new ImageLoaderDDS(); break;
			case ImageFormat::EXR: Loader = new ImageLoaderEXR(); break;
			case ImageFormat::HDR: Loader = new ImageLoaderHDR(); break;
			case ImageFormat::PNG: Loader = new ImageLoaderPNG(); break;
			case ImageFormat::TIF: Loader = new ImageLoaderTIF(); break;
			case ImageFormat::JPG: Loader = new ImageLoaderJPG(); break;
			case ImageFormat::TGA: Loader = new ImageLoaderTGA(); break;
			case ImageFormat::Unknown: return false; break;
			default: return false; break;
		}

		if (Loader != nullptr)
		{
			if (!Loader->Load(InFileName.data()))
			{
				delete Loader;
				return false;
			}

			Data    = Loader->Data;
			Width   = Loader->Width;
			Height  = Loader->Height;
			MipMaps = Loader->Mipmaps;
			Format  = Loader->Format;

			switch (Loader->ImageType)
			{
			case ImageLoader::Type::Image2D:      ImageType = Type::Image2D;      break;
			case ImageLoader::Type::Image3D:      ImageType = Type::Image3D;      break;
			case ImageLoader::Type::ImageCube:    ImageType = Type::ImageCube;    break;
			case ImageLoader::Type::Image2DArray: ImageType = Type::Image2DArray; break;
			}

			delete Loader;
		}

		if (Data == nullptr)
		{
			return false;
		}

		Exist = true;

		switch (Flags)
		{
			case ImageLoading::FlipX:  FlipX();  break;
			case ImageLoading::FlipY:  FlipY();  break;
			case ImageLoading::FlipXY: FlipXY(); break;
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
	bool Image::Save(const char* InFileName, ImageFormat InFormat, uint32 Quality) const
	{
		return false;

		if (!IsExist())
		{
			return false;
		}

		switch (InFormat)
		{
		case ImageFormat::BMP: return ImageSaveBMP(InFileName, Width, Height, Format, Data);          break;
		case ImageFormat::JPG: return ImageSaveJPG(InFileName, Width, Height, Format, Data, Quality); break;
		case ImageFormat::PNG: return ImageSavePNG(InFileName, Width, Height, Format, Data);          break;
		case ImageFormat::TGA: return ImageSaveTGA(InFileName, Width, Height, Format, Data);          break;
		case ImageFormat::TIF: return ImageSaveTIF(InFileName, Width, Height, Format, Data);          break;
		default: break;
		}
		
		return false;
	}
	/*
	* Checks if image is exist in memory
	*/
	bool Image::IsExist() const
	{
		return Exist;
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
	}
	/*
	* Horizontal image flipping
	*/
	bool Image::FlipX()
	{
		if (IsExist())
		{
			return ImageFlipX(Data, Width, Height, GetBPPFromFormat(Format));
		}

		return false;
	}
	/*
	* Vertical image flipping
	*/
	bool Image::FlipY()
	{
		if (IsExist())
		{
			return ImageFlipY(Data, Width, Height, GetBPPFromFormat(Format));
		}

		return false;
	}
	/*
	* Diagonal image flipping
	*/
	bool Image::FlipXY()
	{
		if (IsExist())
		{
			return ImageFlipXY(Data, Width, Height, GetBPPFromFormat(Format));
		}

		return false;
	}

	bool Image::IsRawFormat() const
	{
		return ImageIsRawFormat(Format);
	}

	bool Image::IsUnsignedShortFormat() const
	{
		return ImageIsUnsignedShortFormat(Format);
	}

	bool Image::IsHalfFormat() const
	{
		return ImageIsHalfFormat(Format);
	}

	bool Image::IsFloatFormat() const
	{
		return ImageIsFloatFormat(Format);
	}

	bool Image::IsCompressedFormat() const
	{
		return ImageIsCompressedFormat(Format);
	}

	Image::Type Image::GetType() const
	{
		return ImageType;
	}
	
	uint32 Image::GetWidth() const
	{
		return Width;
	}
	
	uint32 Image::GetHeight() const
	{
		return Height;
	}

	uint32 Image::GetDepth() const
	{
		return Depth;
	}

	uint32 Image::GetMipmapsCount() const
	{
		return MipMaps;
	}

	uint32 Image::GetBytesPerPixel() const
	{
		return GetBPPFromFormat(Format);
	}

	uint32 Image::GetBytesPerBlock() const
	{
		return GetBlockSizeFromFormat(Format);
	}

	uint64 Image::GetOffset(uint32 Level) const
	{
		if (IsCompressedFormat())
		{
			if (Level < MipMaps)
			{
				uint64 Offset = 0;
				uint32 BlockSize = 0;

				if (Format == TextureFormat::DXT1)
				{
					BlockSize = 8;
				}
				else
				{
					BlockSize = 16;
				}

				for (uint32 i = 0; i < Level; i++)
				{
					Offset += (((Width >> i) + 3) / 4) * (((Height >> i) + 3) / 4) * BlockSize;
				}

				return Offset;
			}
		}

		return 0;
	}

	uint64 Image::GetSize(uint32 Level) const
	{
		if (IsCompressedFormat())
		{
			uint32 BlockSize = 0;

			if (Format == TextureFormat::DXT1)
			{
				BlockSize = 8;
			}
			else
			{
				BlockSize = 16;
			}

			return (((Width >> Level) + 3) / 4) * (((Height >> Level) + 3) / 4) * BlockSize;
		}
		else
		{
			return (Width >> Level) * (Height >> Level) * GetBytesPerPixel();
		}

		return 0;
	}

	size_t Image::GetNumChannels() const
	{
		return ImageGetNumChannelsFromFormat(Format);
	}

	uint8* Image::Get2DData(uint32 Level) const
	{
		if (ImageType == Type::Image2D)
		{
			if (Data != nullptr)
			{
				return &Data[0] + GetOffset(Level);
			}
		}

		return nullptr;
	}

	uint8* Image::GetCubeData(uint32 Face, uint32 Level) const
	{
		if (ImageType == Type::ImageCube)
		{
			if (Data != nullptr)
			{
				uint64 FaceSize = 0;
				for (uint32 i = 0; i < MipMaps; i++) FaceSize += GetSize(i);
				return &Data[0] + Face * FaceSize + GetOffset(Level);
			}
		}

		return nullptr;
	}

	TextureFormat Image::GetFormat() const
	{
		return Format;
	}
	
	uint8* Image::GetData() const
	{
		return Data;
	}
	
	Image::~Image()
	{
		FreeData();
	}
}






