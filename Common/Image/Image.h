#pragma once

#include <Core/Types.h>
#include <System/File.h>
#include <cstring>
#include <string_view>

namespace Columbus
{

	class Image;

	enum class ImageLoading
	{
		None,
		FlipX,
		FlipY,
		FlipXY
	};

	/**
	* @brief Format of stored on disk image.
	*/
	enum class ImageFormat
	{
		BMP,
		DDS,
		EXR,
		PNG,
		TIF,
		JPG,
		TGA,
		HDR,
		Unknown
	};

	/**
	* @brief GPU texture format. If you add format, don't forget to update Image.cpp formats array
	*/
	enum class TextureFormat
	{
		R8,
		RG8,
		RGB8,
		RGBA8,
		BGRA8,

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

		R8UInt,
		R32UInt,

		R8SRGB,
		RG8SRGB,
		RGB8SRGB,
		RGBA8SRGB,
		BGRA8SRGB,

		R11G11B10F,

		DXT1,
		DXT3,
		DXT5,
		BC6H,
		BC7,

		Depth16,
		Depth24,
		Depth24Stencil8,
		Depth32F,
		Depth32FStencil8,
		Unknown
	};

	enum class ImageType
	{
		Image2D,
		Image3D,
		ImageCube,
		Image2DArray
	};

	// obtained by TextureFormatGetInfo
	struct TextureFormatInfo
	{
		const char* FriendlyName;

		TextureFormat Format;

		u8 BitsPerPixel;
		u8 CompressedBlockSizeBits;
		u8 NumChannels;

		u8 HasAlpha : 1;
		u8 HasCompression : 1;
		u8 HasDepth : 1;
		u8 HasStencil : 1;
	};

	const TextureFormatInfo& TextureFormatGetInfo(TextureFormat Format);

	namespace ImageUtils
	{
		u64 ImageCalcByteSizeForMip(u32 Width, u32 Height, u32 Depth, u32 Mip, TextureFormat Format);

		u64 ImageCalcByteSize(u32 Width, u32 Height, u32 Depth, u32 Mips, TextureFormat Format);
		u64 ImageCalcByteSize(u32 Width, u32 Height, u32 Depth, u32 Mips, u32 Layers, TextureFormat Format);
		u64 ImageCalcByteSize(u32 Width, u32 Height, u32 Depth, u32 Mips, TextureFormat Format, ImageType Type);

		ImageFormat ImageGetFileFormatFromStream(DataStream& Stream);
		bool ImageCheckFormatFromStreamBMP(DataStream& Stream);
		bool ImageCheckFormatFromStreamTGA(DataStream& Stream); // crappy test, so use it last!
		bool ImageCheckFormatFromStreamPNG(DataStream& Stream);
		bool ImageCheckFormatFromStreamTIF(DataStream& Stream);
		bool ImageCheckFormatFromStreamJPG(DataStream& Stream);
		bool ImageCheckFormatFromStreamEXR(DataStream& Stream);
		bool ImageCheckFormatFromStreamHDR(DataStream& Stream);
		bool ImageCheckFormatFromStreamDDS(DataStream& Stream);

		// auto detects file format
		bool ImageLoadFromStream   (DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutDepth, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData);
		bool ImageLoadFromStreamBMP(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData);
		bool ImageLoadFromStreamTGA(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData);
		bool ImageLoadFromStreamPNG(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData);
		bool ImageLoadFromStreamTIF(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData);
		bool ImageLoadFromStreamJPG(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData);
		bool ImageLoadFromStreamEXR(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData);
		bool ImageLoadFromStreamHDR(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData);
		bool ImageLoadFromStreamDDS(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutDepth, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData);

		bool ImageSaveToStreamDDS(DataStream& Stream, const Image& Img);

		bool ImageSaveToFileBMP(const char* FileName, u32 Width, u32 Height, TextureFormat Format, u8* Data);
		bool ImageSaveToFileTGA(const char* FileName, u32 Width, u32 Height, TextureFormat Format, u8* Data);
		bool ImageSaveToFilePNG(const char* FileName, u32 Width, u32 Height, TextureFormat Format, u8* Data);
		bool ImageSaveToFileTIF(const char* FileName, u32 Width, u32 Height, TextureFormat Format, u8* Data);
		bool ImageSaveToFileJPG(const char* FileName, u32 Width, u32 Height, TextureFormat Format, u8* Data, u32 Quality = 100);

		bool ImageBGR2RGB  (u8* Data, u64 Size);
		bool ImageBGRA2RGBA(u8* Data, u64 Size);
		bool ImageABGR2RGBA(u8* Data, u64 Size);
		bool ImageRGB2BGR  (u8* Data, u64 Size);
		bool ImageRGBA2BGRA(u8* Data, u64 Size);

		bool ImageFlipX (u8* Data, u32 Width, u32 Height, u32 BPP);
		bool ImageFlipY (u8* Data, u32 Width, u32 Height, u32 BPP);
		bool ImageFlipXY(u8* Data, u32 Width, u32 Height, u32 BPP);
	}

	struct ImageMip
	{
		u32 Width;
		u32 Height;
		u32 Depth;
		TextureFormat Format;
		u8* Data;

	public:
		// for any read/write function:
		// applies format conversion automatically
		// supports limited number of formats
		// will return false if conversion failed

		// works with WxHxD pixels from the image, starting from [X, Y, Z] coordinate
		//bool ReadPixelsRGBA32F(float* Pixels, u32 X, u32 Y, u32 Z, u32 W, u32 H, u32 D);

		bool ReadPixelRGBA32F(float Pixels[4], u32 X, u32 Y, u32 Z) const;
		bool WritePixelRGBA32F(float Pixels[4], u32 X, u32 Y, u32 Z) const;
	};

	/**
	* @brief CPU-side image.
	*/
	class Image
	{
	public:
		u32 Width = 0;
		u32 Height = 0;
		u32 Depth = 0;
		u32 MipMaps = 0;
		u64 Size = 0;
		TextureFormat Format = TextureFormat::RGBA8;
		u8* Data = nullptr;
		bool Exist = false;

		ImageType Type;
	public:
		Image() {}
		Image(std::string_view FileName) { if (!LoadFromFile(FileName)) { Data = nullptr; Exist = false; } }
		Image(const Image&) = delete;
		Image(Image&& Base) noexcept { *this = (Image&&)(Base); }

		Image& operator=(const Image& Other)
		{
			FreeData();
			Width = Other.Width;
			Height = Other.Height;
			Depth = Other.Depth;
			Format = Other.Format;
			Exist = Other.Exist;
			Type = Other.Type;

			if (Other.Data)
			{
				u64 DataSize = ImageUtils::ImageCalcByteSize(Width, Height, Depth, MipMaps, Format) * (Type == ImageType::ImageCube ? 6 : 1);
				Data = new u8[DataSize];
				memcpy(Data, Other.Data, DataSize);
			}

			return *this;
		}

		Image& operator=(Image&& Base) noexcept
		{
			auto swap = [](auto& a, auto& b)
			{
				auto tmp = a;
				a = b;
				b = tmp;
			};

			swap(Width, Base.Width);
			swap(Height, Base.Height);
			swap(Depth, Base.Depth);
			swap(MipMaps, Base.MipMaps);
			swap(Size, Base.Size);
			swap(Format, Base.Format);
			swap(Data, Base.Data);
			swap(Exist, Base.Exist);
			swap(Type, Base.Type);
			return *this;
		}

		// doesn't decode image from memory, initialises from raw data
		void FromMemory(const void* InData, int InSize, int W, int H)
		{
			Data = new uint8[InSize];
			Size = InSize;
			Width = W;
			Height = H;
			memcpy(this->Data, InData, Size);
			Type = ImageType::Image2D;
			Exist = true;
			MipMaps = 1;
		}

		// will empty internals
		// used for empty image initialisation
		void AllocImage(u32 W, u32 H, u32 D, u32 Mips, TextureFormat Format, ImageType Type);

		ImageMip GetMip(u32 Mip, u32 Layer) const;

		bool LoadFromFile(std::string_view FileName, ImageLoading Flags = ImageLoading::None);
		bool LoadFromMemory(const u8* Memory, const u64 Size, ImageLoading Flags = ImageLoading::None);
		bool LoadFromStream(DataStream& Stream, ImageLoading Flags = ImageLoading::None);

		bool SaveToFile(const char* FileName, ImageFormat Format, u32 Quality = 100) const;
		void FreeData();

		ImageType GetType() const { return Type; }

		bool IsExist() const { return Exist; }

		u32 GetWidth () const { return Width;  }
		u32 GetHeight() const { return Height; }
		u32 GetDepth () const { return Depth;  }

		u32 GetMipmapsCount() const { return MipMaps; }

		u64 GetOffset(u32 Layer, u32 Level) const;
		u64 GetSize(u32 Level) const;

		u64 GetTotalPixelCount() const;
		u64 GetFullSize() const;

		u8* Get2DData(u32 Level = 0) const;
		u8* GetCubeData(u32 Face, u32 Level = 0) const;

		TextureFormat GetFormat() const { return Format; }
		u8* GetData() const { return Data; }

		~Image();
	};

	namespace ImageMips
	{
		struct MipGenerationParams
		{
			
		};

		void GenerateImageMips(const Image& Img, Image& DstImage, MipGenerationParams Params);
	}

	namespace ImageCompression
	{
		struct CompressionParams
		{
			TextureFormat Format;

			// BC7 only
			bool PerceptualMetric = true;
		};

		// init must be called once in the beginning
		void InitImageCompression();

		bool CompressImageBcEnc(const Image& Img, Image& DstImage, CompressionParams Params);
		bool CompressImageCompressonator(const Image& Img, Image& DstImage, CompressionParams Params);
		bool CompressImage(const Image& Img, Image& DstImage, CompressionParams Params);

		// selects the most suitable decompressed format
		// returns true if decompression was performed
		bool DecompressImage(const Image& Img, Image& DstImage);
	}

}
