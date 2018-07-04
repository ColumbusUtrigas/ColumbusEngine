#pragma once

#include <System/Assert.h>
#include <System/System.h>
#include <Core/Types.h>
#include <string>
#include <cstdlib>
#include <cstring>

namespace Columbus
{

	enum class ImageLoading
	{
		None,
		FlipX,
		FlipY,
		FlipXY
	};

	enum class ImageFormat
	{
		BMP,
		DDS,
		PNG,
		TIF,
		JPG,
		TGA,
		Unknown
	};

	enum class TextureFormat
	{
		R8,
		RG8,
		RGB8,
		RGBA8,

		R16,
		RG16,
		RGB16,
		RGBA16,

		R16F,
		RG16F,
		RGB16F,
		RGBA16F,

		R32F,
		RG32F,
		RGB32F,
		RGBA32F,

		DXT1,
		DXT3,
		DXT5,

		Depth,
		Depth16,
		Depth24,
		Depth24Stencil8,
		Depth32F,
		Depth32FStencil8,
		Unknown
	};

	class ImageLoader
	{
	protected:
		uint8* Data = nullptr;
		uint32 Width = 0;
		uint32 Height = 0;
		uint32 Mipmaps = 0;
		TextureFormat Format = TextureFormat::RGBA8;
	public:
		ImageLoader() {}

		virtual bool Load(std::string FileName) { return false; }
		virtual void Free() {}

		uint8* GetData() const { return Data; }
		uint32 GetWidth() const { return Width; }
		uint32 GetHeight() const { return Height; }
		uint32 GetMipmaps() const { return Mipmaps; }
		TextureFormat GetFormat() const { return Format; }

		virtual ~ImageLoader() {}
	};

	ImageFormat ImageGetFormat(std::string FileName);
	uint32 GetBPPFromFormat(TextureFormat Format);

	bool ImageSaveBMP(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSaveTGA(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSavePNG(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSaveTIF(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSaveJPG(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data, uint32 Quality = 100);

	bool ImageSave(std::string FileName, uint32 Width, uint32 Height, TextureFormat BPP, uint8* Data, ImageFormat Format, uint32 Quality = 100);

	bool ImageBGR2RGB(uint8* Data, uint64 Size);
	bool ImageBGRA2RGBA(uint8* Data, uint64 Size);
	bool ImageABGR2RGBA(uint8* Data, uint64 Size);
	bool ImageRGB2BGR(uint8* Data, uint64 Size);
	bool ImageRGBA2BGRA(uint8* Data, uint64 Size);

	bool ImageFlipX(uint8* Data, uint32 Width, uint32 Height, uint32 BPP);
	bool ImageFlipY(uint8* Data, uint32 Width, uint32 Height, uint32 BPP);
	bool ImageFlipXY(uint8* Data, uint32 Width, uint32 Height, uint32 BPP);

	class Image
	{
	public:
		enum class Type;
	private:
		uint32 Width = 0;         //Width of the image
		uint32 Height = 0;        //Height of the image
		uint32 Depth = 0;
		uint64 Size = 0;
		uint32 MipMaps = 0;
		TextureFormat Format = TextureFormat::RGBA8;
		uint8* Data = nullptr;    //Pixel data
		bool Exist = false;       //Is image exist

		Type ImageType;

		std::string FileName;
	public:
		enum class Type
		{
			Image2D,
			Image3D,
			ImageCube,
			Image2DArray
		};
	public:
		Image();

		bool Load(std::string InFilename, ImageLoading Flags = ImageLoading::None);
		bool Save(std::string InFilename, ImageFormat Format, size_t Quality = 100) const;
		bool IsExist() const;
		void FreeData();

		bool FlipX();
		bool FlipY();
		bool FlipXY();

		bool IsRawFormat() const;
		bool IsUnsignedShortFormat() const;
		bool IsHalfFormat() const;
		bool IsFloatFormat() const;
		bool IsCompressedFormat() const;

		Type GetType() const;

		uint32 GetWidth() const;
		uint32 GetHeight() const;
		uint32 GetDepth() const;
		uint32 GetMipmapsCount() const;
		uint32 GetBytesPerPixel() const;
		uint32 GetBytesPerBlock() const;

		uint64 GetOffset(uint32 Level) const;
		uint64 GetSize(uint32 Level) const;
		//uint64 GetSize() const;

		uint8* Get2DData(uint32 Level = 0) const;
		uint8* GetCubeData(uint32 Face, uint32 Level = 0) const;

		TextureFormat GetFormat() const;
		uint8* GetData() const;
		std::string GetFileName() const;

		~Image();
	};

}



