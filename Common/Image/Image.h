#pragma once

#include <Core/Types.h>
#include <utility>

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
		HDR,
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

		R11G11B10F,

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
	public:
		enum class Type;
	public:
		uint32 Width = 0;
		uint32 Height = 0;
		uint32 Mipmaps = 0;

		TextureFormat Format = TextureFormat::RGBA8;
		Type ImageType;

		uint8* Data = nullptr;
	public:
		enum class Type
		{
			Image2D,
			Image3D,
			ImageCube,
			Image2DArray
		};
	public:
		ImageLoader() {}

		virtual bool Load(const char* FileName) { return false; }

		virtual ~ImageLoader() {}
	};

	ImageFormat ImageGetFormat(const char* FileName);
	uint32 GetBPPFromFormat(TextureFormat Format);
	uint32 GetBlockSizeFromFormat(TextureFormat Format);

	bool ImageSaveBMP(const char* FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSaveTGA(const char* FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSavePNG(const char* FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSaveTIF(const char* FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSaveJPG(const char* FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data, uint32 Quality = 100);

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
		uint32 Width = 0;
		uint32 Height = 0;
		uint32 Depth = 0;
		uint32 MipMaps = 0;
		uint64 Size = 0;
		TextureFormat Format = TextureFormat::RGBA8;
		uint8* Data = nullptr;
		bool Exist = false;

		Type ImageType;
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
		Image(const char* FileName) { if (!Load(FileName)) { Data = nullptr; Exist = false; } }
		Image(const Image&) = delete;
		Image(Image&& Base) noexcept { *this = std::move(Base); }

		Image& operator=(const Image&) = delete;
		Image& operator=(Image&& Base) noexcept
		{
			std::swap(Width, Base.Width);
			std::swap(Height, Base.Height);
			std::swap(Depth, Base.Depth);
			std::swap(MipMaps, Base.MipMaps);
			std::swap(Size, Base.Size);
			std::swap(Format, Base.Format);
			std::swap(Data, Base.Data);
			std::swap(Exist, Base.Exist);
			return *this;
		}

		bool Load(const char* FileName, ImageLoading Flags = ImageLoading::None);
		bool Save(const char* FileName, ImageFormat Format, uint32 Quality = 100) const;
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

		uint8* Get2DData(uint32 Level = 0) const;
		uint8* GetCubeData(uint32 Face, uint32 Level = 0) const;

		TextureFormat GetFormat() const;
		uint8* GetData() const;

		~Image();
	};

}


