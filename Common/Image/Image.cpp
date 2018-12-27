#include <Common/Image/Image.h>
#include <Common/Image/BMP/ImageBMP.h>
#include <Common/Image/DDS/ImageDDS.h>
#include <Common/Image/JPG/ImageJPG.h>
#include <Common/Image/PNG/ImagePNG.h>
#include <Common/Image/TGA/ImageTGA.h>
#include <Common/Image/TIF/ImageTIF.h>
#include <Core/Core.h>

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

			case TextureFormat::Unknown: return 0;  break;

			default:                     return 0;  break;
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
		}

		return 0;
	}

	ImageFormat ImageGetFormat(std::string FileName)
	{
		if (ImageLoaderBMP::IsBMP(FileName)) return ImageFormat::BMP;
		if (ImageLoaderDDS::IsDDS(FileName)) return ImageFormat::DDS;
		if (ImageLoaderPNG::IsPNG(FileName)) return ImageFormat::PNG;
		if (ImageLoaderTIF::IsTIF(FileName)) return ImageFormat::TIF;
		if (ImageLoaderJPG::IsJPG(FileName)) return ImageFormat::JPG;
		if (ImageLoaderTGA::IsTGA(FileName)) return ImageFormat::TGA;

		return ImageFormat::Unknown;
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
				Memory::Memcpy(pixel, &row[j * BPP], BPP);
				Memory::Memcpy(&row[j * BPP], &row[(Width - j) * BPP], BPP);
				Memory::Memcpy(&row[(Width - j) * BPP], pixel, BPP);
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
			Memory::Memcpy(row, &Data[i * stride], stride);
			Memory::Memcpy(&Data[i * stride], &Data[(Height - i - 1) * stride], stride);
			Memory::Memcpy(&Data[(Height - i - 1) * stride], row, stride);
		}
		Memory::Free(row);

		return true;
	}
	
	bool ImageFlipXY(uint8* Data, uint32 Width, uint32 Height, uint32 BPP)
	{
		if (Data == nullptr) return false;

		const uint32 size = Width * Height;
		uint8* pixel = new uint8[BPP];

		for (size_t i = 0; i < size / 2; i++)
		{
			Memory::Memcpy(pixel, &Data[i * BPP], BPP);
			Memory::Memcpy(&Data[i * BPP], &Data[(size - i) * BPP], BPP);
			Memory::Memcpy(&Data[(size - i) * BPP], pixel, BPP);
		}
		Memory::Free(pixel);

		return true;
	}
	
	bool ImageSave(std::string FileName, uint32 Width, uint32 Height, TextureFormat BPP, uint8* Data, ImageFormat Format, uint32 Quality)
	{
		switch (Format)
		{
		case ImageFormat::BMP:
			return ImageSaveBMP(FileName, Width, Height, BPP, Data);
			break;
		case ImageFormat::DDS:
			return false; //TODO
			break;
		case ImageFormat::TGA:
			return ImageSaveTGA(FileName, Width, Height, BPP, Data);
			break;
		case ImageFormat::PNG:
			return ImageSavePNG(FileName, Width, Height, BPP, Data);
			break;
		case ImageFormat::TIF:
			return ImageSaveTIF(FileName, Width, Height, BPP, Data);
			break;
		case ImageFormat::JPG:
			return ImageSaveJPG(FileName, Width, Height, BPP, Data, Quality);
			break;
		case ImageFormat::Unknown:
			return false;
			break;
		}

		return false;
	}
	/*
	* Image class
	*/
	Image::Image() :
		Width(0),
		Height(0),
		Depth(0),
		Size(0),
		MipMaps(0),
		Format(TextureFormat::RGBA8),
		Data(nullptr),
		Exist(false) {}
	/*
	* Load image from file
	* @param std::string InFileName: Name of image file to load
	* @param int Flags: Loading flags
	*/
	bool Image::Load(std::string InFileName, ImageLoading Flags)
	{
		FreeData();

		ImageLoader* Loader = nullptr;

		switch (ImageGetFormat(InFileName))
		{
			case ImageFormat::BMP: Loader = new ImageLoaderBMP(); break;
			case ImageFormat::DDS: Loader = new ImageLoaderDDS(); break;
			case ImageFormat::PNG: Loader = new ImageLoaderPNG(); break;
			case ImageFormat::TIF: Loader = new ImageLoaderTIF(); break;
			case ImageFormat::JPG: Loader = new ImageLoaderJPG(); break;
			case ImageFormat::TGA: Loader = new ImageLoaderTGA(); break;
			case ImageFormat::Unknown: return false; break;
			default: return false; break;
		}

		if (Loader != nullptr)
		{
			if (!Loader->Load(InFileName))
			{
				delete Loader;
				return false;
			}

			Data    = std::move(Loader->Data);
			Width   = std::move(Loader->Width);
			Height  = std::move(Loader->Height);
			MipMaps = std::move(Loader->Mipmaps);
			Format  = std::move(Loader->Format);

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

		FileName = InFileName;
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
	* @param std::string InFileName: Name of image file to save
	* @param Format: Image format
	* @param Quality: Compression level
	*/
	bool Image::Save(std::string InFileName, ImageFormat InFormat, size_t Quality) const
	{
		return false;

		if (!IsExist())
		{
			return false;
		}

		return ImageSave(InFileName, Width, Height, Format, Data, InFormat, Quality);
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
		FileName.clear();

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
		return (Format == TextureFormat::R8 ||
		        Format == TextureFormat::RG8 ||
		        Format == TextureFormat::RGB8 ||
		        Format == TextureFormat::RGBA8);
	}

	bool Image::IsUnsignedShortFormat() const
	{
		return (Format == TextureFormat::R16 ||
		        Format == TextureFormat::RG16 ||
		        Format == TextureFormat::RGB16 ||
		        Format == TextureFormat::RGBA16);
	}

	bool Image::IsHalfFormat() const
	{
		return (Format == TextureFormat::R16F ||
		        Format == TextureFormat::RG16F ||
		        Format == TextureFormat::RGB16F ||
		        Format == TextureFormat::RGBA16F);
	}

	bool Image::IsFloatFormat() const
	{
		return (Format == TextureFormat::R32F ||
		        Format == TextureFormat::RG32F ||
		        Format == TextureFormat::RGB32F ||
		        Format == TextureFormat::RGBA32F);
	}

	bool Image::IsCompressedFormat() const
	{
		return (Format == TextureFormat::DXT1 ||
		        Format == TextureFormat::DXT3 ||
		        Format == TextureFormat::DXT5);
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
		switch (Format)
		{
			case TextureFormat::DXT1: return 8;  break;
			case TextureFormat::DXT3: return 16; break;
			case TextureFormat::DXT5: return 16; break;

			case TextureFormat::R8:
			case TextureFormat::RG8:
			case TextureFormat::RGB8:
			case TextureFormat::RGBA8:
			case TextureFormat::R16:
			case TextureFormat::RG16:
			case TextureFormat::RGB16:
			case TextureFormat::RGBA16:
			case TextureFormat::R16F:
			case TextureFormat::RG16F:
			case TextureFormat::RGB16F:
			case TextureFormat::RGBA16F:
			case TextureFormat::R32F:
			case TextureFormat::RG32F:
			case TextureFormat::RGB32F:
			case TextureFormat::RGBA32F:
			case TextureFormat::Depth:
			case TextureFormat::Depth16:
			case TextureFormat::Depth24:
			case TextureFormat::Depth24Stencil8:
			case TextureFormat::Depth32F:
			case TextureFormat::Depth32FStencil8:
			case TextureFormat::Unknown: return 0; break;
		}

		return 0;
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
	
	std::string Image::GetFileName() const
	{
		return FileName;
	}
	
	Image::~Image()
	{
		FreeData();
	}
}






