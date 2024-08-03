#include <Common/Image/Image.h>
#include <System/File.h>
#include <System/Log.h>
#include <Core/Assert.h>
#include <Math/MathUtil.h>
#include <cstring>

namespace Columbus::ImageUtils
{

	/*
	* Surface description flags
	*/
	constexpr uint32 DDSFlagCaps = 0x00000001;
	constexpr uint32 DDSFlagHeight = 0x00000002;
	constexpr uint32 DDSFlagWidth = 0x00000004;
	constexpr uint32 DDSFlagPitch = 0x00000008;
	constexpr uint32 DDSFlagPixelFormat = 0x00001000;
	constexpr uint32 DDSFlagMipMapCount = 0x00020000;
	constexpr uint32 DDSFlagLinearSize = 0x00080000;
	constexpr uint32 DDSFlagDepth = 0x00800000;
	/*
	* Pixel format flags
	*/
	constexpr uint32 DDSAlphaPixels = 0x00000001;
	constexpr uint32 DDSFourCC = 0x00000004;
	constexpr uint32 DDSRGB = 0x00000040;
	constexpr uint32 DDSRGBA = 0x00000041;
	/*
	* Caps1 flags
	*/
	constexpr uint32 DDSComplex = 0x00000008;
	constexpr uint32 DDSTexture = 0x00001000;
	constexpr uint32 DDSMipMap = 0x00400000;
	/*
	* Caps2 flags
	*/
	constexpr uint32 DDSCubemap = 0x00000200;
	constexpr uint32 DDSCubemapPositiveX = 0x00000400;
	constexpr uint32 DDSCubemapNegativeX = 0x00000800;
	constexpr uint32 DDSCubemapPositiveY = 0x00001000;
	constexpr uint32 DDSCubemapNegativeY = 0x00002000;
	constexpr uint32 DDSCubemapPositiveZ = 0x00004000;
	constexpr uint32 DDSCubemapNegativeZ = 0x00008000;
	constexpr uint32 DDSCubemapAllFaces = 0x0000FC00;
	constexpr uint32 DDSVolume = 0x00200000;
	/*
	* Compression types
	*/
	//constexpr uint32 DDSFourCC_DXT1 = 0x31545844; // "DXT1", BC1_UNORM
	//constexpr uint32 DDSFourCC_DXT2 = 0x32545844; // "DXT2", BC2_UNORM
	//constexpr uint32 DDSFourCC_DXT3 = 0x33545844; // "DXT3", BC2_UNORM
	//constexpr uint32 DDSFourCC_DXT4 = 0x34545844; // "DXT4", BC3_UNORM
	//constexpr uint32 DDSFourCC_DXT5 = 0x35545844; // "DXT5", BC3_UNORM
	#define	DDS_FOURCC(chars) ((u32)chars[0] | ((u32)chars[1] << 8) | ((u32)chars[2] << 16) | ((u32)chars[3] << 24))
	constexpr uint32 DDSFourccARGB16F = 113; // D3DFMT_A16B16G16R16F
	constexpr uint32 DDSFourccARGB32F = 116; // D3DFMT_A32B32G32R32F

	/*
	* Texture dimension
	*/
	constexpr uint32 DDSDimensionUnkown = 0;
	constexpr uint32 DDSDimensionBuffer = 1;
	constexpr uint32 DDSDimensionTexture1D = 2;
	constexpr uint32 DDSDimensionTexture2D = 3;
	constexpr uint32 DDSDimensionTexture3D = 4;

	/*
	* DX10 Misc flags
	*/
	constexpr uint32 DDS_DX10_Misc_Cube = 0x4;
	constexpr uint32 DDS_DX10_Misc2_AlphaUnkown = 0x0;
	constexpr uint32 DDS_DX10_Misc2_AlphaStraight = 0x1;
	constexpr uint32 DDS_DX10_Misc2_AlphaPremultiplied = 0x2;
	constexpr uint32 DDS_DX10_Misc2_AlphaOpaque = 0x3;
	constexpr uint32 DDS_DX10_Misc2_AlphaCustom = 0x4;

	TextureFormat DdsDxgiFormat_Map[] =
	{
		TextureFormat::Unknown,    // DXGI_FORMAT_UNKNOWN = 0,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
		TextureFormat::RGBA32F,    // DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32G32B32A32_UINT = 3,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32G32B32A32_SINT = 4,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32G32B32_TYPELESS = 5,
		TextureFormat::RGB32F,     // DXGI_FORMAT_R32G32B32_FLOAT = 6,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32G32B32_UINT = 7,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32G32B32_SINT = 8,
		TextureFormat::Unknown,    // DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
		TextureFormat::RGBA16F,    // DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
		TextureFormat::RGBA16,     // DXGI_FORMAT_R16G16B16A16_UNORM = 11,
		TextureFormat::RGBA16,     // DXGI_FORMAT_R16G16B16A16_UINT = 12,
		TextureFormat::RGBA16,     // DXGI_FORMAT_R16G16B16A16_SNORM = 13,
		TextureFormat::RGBA16,     // DXGI_FORMAT_R16G16B16A16_SINT = 14,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32G32_TYPELESS = 15,
		TextureFormat::RG32F,      // DXGI_FORMAT_R32G32_FLOAT = 16,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32G32_UINT = 17,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32G32_SINT = 18,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32G8X24_TYPELESS = 19,
		TextureFormat::Unknown,    // DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
		TextureFormat::Unknown,    // DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
		TextureFormat::Unknown,    // DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
		TextureFormat::Unknown,    // DXGI_FORMAT_R10G10B10A2_UNORM = 24,
		TextureFormat::Unknown,    // DXGI_FORMAT_R10G10B10A2_UINT = 25,
		TextureFormat::R11G11B10F, // DXGI_FORMAT_R11G11B10_FLOAT = 26,
		TextureFormat::RGBA8,      // DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
		TextureFormat::RGBA8,      // DXGI_FORMAT_R8G8B8A8_UNORM = 28,
		TextureFormat::RGBA8SRGB,  // DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
		TextureFormat::RGBA8,      // DXGI_FORMAT_R8G8B8A8_UINT = 30,
		TextureFormat::RGBA8,      // DXGI_FORMAT_R8G8B8A8_SNORM = 31,
		TextureFormat::RGBA8,      // DXGI_FORMAT_R8G8B8A8_SINT = 32,
		TextureFormat::RG16,       // DXGI_FORMAT_R16G16_TYPELESS = 33,
		TextureFormat::RG16F,      // DXGI_FORMAT_R16G16_FLOAT = 34,
		TextureFormat::RG16,       // DXGI_FORMAT_R16G16_UNORM = 35,
		TextureFormat::RG16,       // DXGI_FORMAT_R16G16_UINT = 36,
		TextureFormat::RG16,       // DXGI_FORMAT_R16G16_SNORM = 37,
		TextureFormat::RG16,       // DXGI_FORMAT_R16G16_SINT = 38,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32_TYPELESS = 39,
		TextureFormat::Unknown,    // DXGI_FORMAT_D32_FLOAT = 40,
		TextureFormat::R32F,       // DXGI_FORMAT_R32_FLOAT = 41,
		TextureFormat::R32UInt,    // DXGI_FORMAT_R32_UINT = 42,
		TextureFormat::Unknown,    // DXGI_FORMAT_R32_SINT = 43,
		TextureFormat::Unknown,    // DXGI_FORMAT_R24G8_TYPELESS = 44,
		TextureFormat::Unknown,    // DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
		TextureFormat::Unknown,    // DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
		TextureFormat::Unknown,    // DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
		TextureFormat::RG8,        // DXGI_FORMAT_R8G8_TYPELESS = 48,
		TextureFormat::RG8,        // DXGI_FORMAT_R8G8_UNORM = 49,
		TextureFormat::RG8,        // DXGI_FORMAT_R8G8_UINT = 50,
		TextureFormat::RG8,        // DXGI_FORMAT_R8G8_SNORM = 51,
		TextureFormat::RG8,        // DXGI_FORMAT_R8G8_SINT = 52,
		TextureFormat::Unknown,    // DXGI_FORMAT_R16_TYPELESS = 53,
		TextureFormat::R16F,       // DXGI_FORMAT_R16_FLOAT = 54,
		TextureFormat::Unknown,    // DXGI_FORMAT_D16_UNORM = 55,
		TextureFormat::R16,        // DXGI_FORMAT_R16_UNORM = 56,
		TextureFormat::R16,        // DXGI_FORMAT_R16_UINT = 57,
		TextureFormat::R16,        // DXGI_FORMAT_R16_SNORM = 58,
		TextureFormat::R16,        // DXGI_FORMAT_R16_SINT = 59,
		TextureFormat::R8,         // DXGI_FORMAT_R8_TYPELESS = 60,
		TextureFormat::R8,         // DXGI_FORMAT_R8_UNORM = 61,
		TextureFormat::R8,         // DXGI_FORMAT_R8_UINT = 62,
		TextureFormat::R8,         // DXGI_FORMAT_R8_SNORM = 63,
		TextureFormat::R8,         // DXGI_FORMAT_R8_SINT = 64,
		TextureFormat::Unknown,    // DXGI_FORMAT_A8_UNORM = 65,
		TextureFormat::Unknown,    // DXGI_FORMAT_R1_UNORM = 66,
		TextureFormat::Unknown,    // DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
		TextureFormat::Unknown,    // DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
		TextureFormat::Unknown,    // DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
		TextureFormat::DXT1,       // DXGI_FORMAT_BC1_TYPELESS = 70,
		TextureFormat::DXT1,       // DXGI_FORMAT_BC1_UNORM = 71,
		TextureFormat::DXT1,       // DXGI_FORMAT_BC1_UNORM_SRGB = 72,
		TextureFormat::DXT3,       // DXGI_FORMAT_BC2_TYPELESS = 73,
		TextureFormat::DXT3,       // DXGI_FORMAT_BC2_UNORM = 74,
		TextureFormat::DXT3,       // DXGI_FORMAT_BC2_UNORM_SRGB = 75,
		TextureFormat::DXT3,       // DXGI_FORMAT_BC3_TYPELESS = 76,
		TextureFormat::DXT3,       // DXGI_FORMAT_BC3_UNORM = 77,
		TextureFormat::DXT3,       // DXGI_FORMAT_BC3_UNORM_SRGB = 78,
		TextureFormat::DXT5,       // DXGI_FORMAT_BC4_TYPELESS = 79,
		TextureFormat::DXT5,       // DXGI_FORMAT_BC4_UNORM = 80,
		TextureFormat::DXT5,       // DXGI_FORMAT_BC4_SNORM = 81,
		TextureFormat::DXT5,       // DXGI_FORMAT_BC5_TYPELESS = 82,
		TextureFormat::DXT5,       // DXGI_FORMAT_BC5_UNORM = 83,
		TextureFormat::DXT5,       // DXGI_FORMAT_BC5_SNORM = 84,
		TextureFormat::Unknown,    // DXGI_FORMAT_B5G6R5_UNORM = 85,
		TextureFormat::Unknown,    // DXGI_FORMAT_B5G5R5A1_UNORM = 86,
		TextureFormat::Unknown,    // DXGI_FORMAT_B8G8R8A8_UNORM = 87,
		TextureFormat::Unknown,    // DXGI_FORMAT_B8G8R8X8_UNORM = 88,
		TextureFormat::Unknown,    // DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
		TextureFormat::BGRA8,      // DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
		TextureFormat::BGRA8SRGB,  // DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
		TextureFormat::Unknown,    // DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
		TextureFormat::Unknown,    // DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
		TextureFormat::BC6H,       // DXGI_FORMAT_BC6H_TYPELESS = 94,
		TextureFormat::BC6H,       // DXGI_FORMAT_BC6H_UF16 = 95,
		TextureFormat::BC6H,       // DXGI_FORMAT_BC6H_SF16 = 96,
		TextureFormat::BC7,        // DXGI_FORMAT_BC7_TYPELESS = 97,
		TextureFormat::BC7,        // DXGI_FORMAT_BC7_UNORM = 98,
		TextureFormat::BC7,        // DXGI_FORMAT_BC7_UNORM_SRGB = 99,
		TextureFormat::Unknown,    // DXGI_FORMAT_AYUV = 100,
		TextureFormat::Unknown,    // DXGI_FORMAT_Y410 = 101,
		TextureFormat::Unknown,    // DXGI_FORMAT_Y416 = 102,
		TextureFormat::Unknown,    // DXGI_FORMAT_NV12 = 103,
		TextureFormat::Unknown,    // DXGI_FORMAT_P010 = 104,
		TextureFormat::Unknown,    // DXGI_FORMAT_P016 = 105,
		TextureFormat::Unknown,    // DXGI_FORMAT_420_OPAQUE = 106,
		TextureFormat::Unknown,    // DXGI_FORMAT_YUY2 = 107,
		TextureFormat::Unknown,    // DXGI_FORMAT_Y210 = 108,
		TextureFormat::Unknown,    // DXGI_FORMAT_Y216 = 109,
		TextureFormat::Unknown,    // DXGI_FORMAT_NV11 = 110,
		TextureFormat::Unknown,    // DXGI_FORMAT_AI44 = 111,
		TextureFormat::Unknown,    // DXGI_FORMAT_IA44 = 112,
		TextureFormat::Unknown,    // DXGI_FORMAT_P8 = 113,
		TextureFormat::Unknown,    // DXGI_FORMAT_A8P8 = 114,
		TextureFormat::Unknown,    // DXGI_FORMAT_B4G4R4A4_UNORM = 115,
		TextureFormat::Unknown,    // DXGI_FORMAT_P208 = 130,
		TextureFormat::Unknown,    // DXGI_FORMAT_V208 = 131,
		TextureFormat::Unknown,    // DXGI_FORMAT_V408 = 132,
		TextureFormat::Unknown,    // DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE,
		TextureFormat::Unknown,    // DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,
		TextureFormat::Unknown,    // DXGI_FORMAT_FORCE_UINT = 0xffffffff
	};

	/*
	* DDS Image pixel format struct
	* @see: https://msdn.microsoft.com/en-us/library/windows/desktop/bb943984(v=vs.85).aspx
	*/
	struct DDS_PIXELFORMAT
	{
		uint32 Size;
		uint32 Flags;
		uint32 FourCC;
		uint32 RGBBitCount;
		uint32 RBitMask;
		uint32 GBitMask;
		uint32 BBitMask;
		uint32 ABitMask;
	};

	/*
	* DDS Image header struct
	* @see: https://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx
	*/
	struct DDS_HEADER
	{
		uint8 Magic[4]; //"DDS " bytes
		uint32 Size;
		uint32 Flags;
		uint32 Height;
		uint32 Width;
		uint32 PitchOrLinearSize;
		uint32 Depth;
		uint32 MipMapCount;
		uint32 Reserved1[11];
		DDS_PIXELFORMAT PixelFormat;
		uint32 Caps1;
		uint32 Caps2;
		uint32 Caps3;
		uint32 Caps4;
		uint32 Reserved2;
	};

	/*
	* DDS DX10 extension header
	* @see: https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-header-dxt10
	*/
	struct DDS_HEADER_DXT10
	{
		u32 Format; // DXGI_FORMAT
		u32 ResourceDimension;
		u32 MiscFlag;
		u32 ArraySize;
		u32 MiscFlags2;
	};

	struct DDS_HEADER_FLAGS
	{
		bool Caps;
		bool Height;
		bool Width;
		bool Pitch;
		bool PixelFormat;
		bool MipMapCount;
		bool LinearSize;
		bool Depth;

		bool Texture;
		bool Mipmap;
		bool Volume;

		void Read(uint32 Flags)
		{
			Caps = Flags & DDSFlagCaps;
			Height = Flags & DDSFlagHeight;
			Width = Flags & DDSFlagWidth;
			Pitch = Flags & DDSFlagPitch;
			PixelFormat = Flags & DDSFlagPixelFormat;
			MipMapCount = Flags & DDSFlagMipMapCount;
			LinearSize = Flags & DDSFlagLinearSize;
			Depth = Flags & DDSFlagDepth;

			Texture = (Flags & DDSFlagCaps) | (Flags & DDSFlagHeight) | (Flags & DDSFlagWidth) | (Flags & DDSFlagPixelFormat);
			Mipmap = Flags & DDSFlagMipMapCount;
			Volume = Flags & DDSFlagDepth;
		}
	};

	static TextureFormat FourCCDecodeTextureFormat(uint32 FourCC)
	{
		switch (FourCC)
		{
		case DDS_FOURCC("DXT1"): return  TextureFormat::DXT1; // BC1_UNORM
		case DDS_FOURCC("DXT2"): // BC2_UNORM
		case DDS_FOURCC("DXT3"): return TextureFormat::DXT3;  // BC2_UNORM
		case DDS_FOURCC("DXT4"): // BC3_UNORM
		case DDS_FOURCC("DXT5"): return TextureFormat::DXT5;  // BC3_UNORM
		case DDS_FOURCC("ATI2"): return TextureFormat::DXT5;  // BC5_UNORM
		case   DDSFourccARGB16F: return TextureFormat::RGBA16F;
		case   DDSFourccARGB32F: return TextureFormat::RGBA32F;
		// TODO: "BC4S" = BC4_SNORM, "BC5S" = BC5_SNORM
		}

		return TextureFormat::Unknown;
	}

	bool ImageCheckFormatFromStreamDDS(DataStream& Stream)
	{
		u8 Magic[4];
		Stream.ReadBytes(Magic, sizeof(Magic));
		Stream.SeekSet(0); // rewind

		if (memcmp(Magic, "DDS ", 4) == 0)
		{
			return true;
		}

		return false;
	}

	bool ImageLoadFromStreamDDS(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutDepth, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData)
	{
		DDS_HEADER Header;
		DDS_HEADER_DXT10 Header10;

		DDS_HEADER_FLAGS HeaderFlags;
		Stream.Read(Header);
		HeaderFlags.Read(Header.Flags);

		bool HasDX10Header = false;
		if ((Header.PixelFormat.Flags & DDSFourCC) && Header.PixelFormat.FourCC == DDS_FOURCC("DX10"))
		{
			Stream.Read(Header10);
			HasDX10Header = true;
		}

		// common
		OutMips = Header.MipMapCount;
		OutWidth = Header.Width;
		OutHeight = Header.Height;
		OutDepth = Header.Depth;
		OutFormat = TextureFormat::Unknown;

		if ((Header.Caps1 & DDSMipMap) == 0 || OutMips == 0)
			OutMips = 1;

		// header parsing
		if (HasDX10Header)
		{
			if (Header10.ArraySize > 1)
			{
				Log::Error("ImageLoadFromStreamDDS() error: Array textures are not supported");
				return false;
			}

			if (Header10.ResourceDimension == DDSDimensionTexture2D)
			{
				OutType = (Header10.MiscFlag & DDS_DX10_Misc_Cube) ? ImageType::ImageCube : ImageType::Image2D;
			}
			else if (Header10.ResourceDimension == DDSDimensionTexture3D)
			{
				OutType = ImageType::Image3D;
			}
			else
			{
				Log::Error("ImageLoadFromStreamDDS() error: Unsupported resource dimension: %i", Header10.ResourceDimension);
				return false;
			}

			OutFormat = DdsDxgiFormat_Map[Header10.Format];

			if (OutFormat == TextureFormat::Unknown)
			{
				Log::Error("ImageLoadFromStreamDDS() error: Unsupported format: %i", Header10.Format);
				return false;
			}
		}
		else // non-DX10 header
		{
			// format parsing
			if (Header.PixelFormat.Flags & DDSFourCC)
			{
				OutFormat = FourCCDecodeTextureFormat(Header.PixelFormat.FourCC);
			}
			else if (Header.PixelFormat.Flags & DDSRGBA ||
			         Header.PixelFormat.Flags & DDSRGB)
			{
				switch (Header.PixelFormat.RGBBitCount)
				{
				case 8:  OutFormat = TextureFormat::R8;    break;
				case 16: OutFormat = TextureFormat::RG8;   break;
				case 24: OutFormat = TextureFormat::RGB8;  break;
				case 32: OutFormat = TextureFormat::RGBA8; break;
				}
			}
			else
			{
				Log::Error("ImageLoadFromStreamDDS() error: Unsupported pixel format, FourCC is %04x", Header.PixelFormat.FourCC);
				return false;
			}

			// image type parsing
			if ((Header.Caps2 & DDSCubemap) && (Header.Caps2 & DDSCubemapAllFaces) == DDSCubemapAllFaces)
			{
				OutType = ImageType::ImageCube;
			}
			else if (Header.Caps2 & DDSVolume && HeaderFlags.Volume)
			{
				OutType = ImageType::Image3D;
			}
			else
			{
				OutType = ImageType::Image2D;
			}
		}
		
		if (OutFormat == TextureFormat::Unknown)
		{
			Log::Error("ImageLoadFromStreamDDS() error: Couldn't parse format from header");
			return false;
		}

		u64 DataSize = ImageUtils::ImageCalcByteSize(OutWidth, OutHeight, OutDepth, OutMips, OutFormat);

		if (OutType == ImageType::ImageCube)
			DataSize *= 6;

		OutData = new u8[DataSize];
		Stream.ReadBytes(OutData, DataSize);

		TextureFormatInfo FormatInfo = TextureFormatGetInfo(OutFormat);

		// apply swizzling
		if (OutFormat == TextureFormat::RG8 || OutFormat == TextureFormat::RGB8 || OutFormat == TextureFormat::RGBA8)
		{
			bool bNeedsSwizzling = false;

			u32 BytesPerPixel = FormatInfo.BitsPerPixel / 8;

			u32 SwizzleR = 0x000000FF;
			u32 SwizzleG = 0x0000FF00;
			u32 SwizzleB = 0x00FF0000;
			u32 SwizzleA = 0xFF000000;
			u32 Swizzles[] = { SwizzleR, SwizzleG, SwizzleB, SwizzleA };
			u32 HeaderSwizzles[] = { Header.PixelFormat.RBitMask, Header.PixelFormat.GBitMask, Header.PixelFormat.BBitMask, Header.PixelFormat.ABitMask };
			
			for (int i = 0; i < FormatInfo.NumChannels; i++)
			{
				if (Swizzles[i] != HeaderSwizzles[i])
				{
					bNeedsSwizzling = true;
				}
			}

			if (bNeedsSwizzling)
			{
				u32 ComponentRemappingIndex[4]{ 0 };

				for (int i = 0; i < FormatInfo.NumChannels; i++)
				{
					u32 Swizzle = HeaderSwizzles[i];
					u32 RightmostSetBit = Swizzle - (Swizzle & Swizzle - 1);
					u32 RightmostBitNumber = (u32)log2(RightmostSetBit);
					u32 ByteNumber = RightmostBitNumber / 8;

					ComponentRemappingIndex[i] = ByteNumber;
				}

				u64 TotalPixels = DataSize / BytesPerPixel;

				for (u32 Pixel = 0; Pixel < TotalPixels; Pixel++)
				{
					u8* CurrentPixel = OutData + Pixel * BytesPerPixel;
					u8 SavedPixel[4]{ 0 };
					memcpy(SavedPixel, CurrentPixel, BytesPerPixel);

					for (int i = 0; i < FormatInfo.NumChannels; i++)
					{
						CurrentPixel[i] = SavedPixel[ComponentRemappingIndex[i]];
					}
				}
			}
		}

		return true;
	}

	bool ImageSaveToStreamDDS(DataStream& Stream, const Image& Img)
	{
		TextureFormat OldHeaderFormats[] =
		{
			TextureFormat::DXT1,
			TextureFormat::DXT3,
			TextureFormat::DXT5,
			TextureFormat::R8,
			TextureFormat::RG8,
			TextureFormat::RGB8,
			TextureFormat::RGBA8,
		};

		DDS_HEADER Header;
		DDS_HEADER_DXT10 Header10;

		TextureFormatInfo FormatInfo = TextureFormatGetInfo(Img.Format);

		// decide if we want to use Header10
		// we want to use it with some formats (BC7 for example), and with array textures
		bool HasDX10Header = false;
		{
			bool OldFormatFound = false;
			for (int i = 0; i < sizeofarray(OldHeaderFormats); i++)
			{
				if (Img.Format == OldHeaderFormats[i])
				{
					OldFormatFound = true;
					break;
				}
			}

			HasDX10Header = !OldFormatFound || Img.Type == ImageType::Image2DArray;
		}

		u32 CompressionFlag = FormatInfo.HasCompression ?  DDSFlagLinearSize : DDSFlagPitch;
		u32 MipFlags = Img.MipMaps > 1 ? DDSFlagMipMapCount : 0;
		u32 DepthFlags = Img.Type == ImageType::Image3D ? DDSFlagDepth : 0;

		// fill in base header
		{
			memcpy(Header.Magic, "DDS ", 4);
			Header.Size = 124;
			Header.Flags = DDSFlagCaps | DDSFlagHeight | DDSFlagWidth | DDSFlagPixelFormat | MipFlags | CompressionFlag | DepthFlags;
			Header.Height = Img.Height;
			Header.Width = Img.Width;

			if (Header.Flags & DDSFlagPitch)
			{
				Header.PitchOrLinearSize = FormatInfo.HasCompression
					? Math::Max(1u, (Img.Width + 3) / 4) * (FormatInfo.CompressedBlockSizeBits / 8)
					: (Img.Width * FormatInfo.BitsPerPixel + 7) / 8;
			}
			else
			{
				Header.PitchOrLinearSize = FormatInfo.BitsPerPixel * Img.Width * Img.Height / 8;
			}

			Header.Depth = Img.Type == ImageType::Image3D ? Img.Depth : 0;
			Header.MipMapCount = Img.MipMaps;
			memset(Header.Reserved1, 0, sizeof(Header.Reserved1));

			Header.PixelFormat.Size = 32;

			if (FormatInfo.HasCompression)
			{
				Header.PixelFormat.Flags = DDSFourCC;
				Header.PixelFormat.RGBBitCount = 0;
				Header.PixelFormat.RBitMask = 0;
				Header.PixelFormat.GBitMask = 0;
				Header.PixelFormat.BBitMask = 0;
				Header.PixelFormat.ABitMask = 0;
			}
			else
			{
				Header.PixelFormat.Flags = DDSRGB | (FormatInfo.HasAlpha ? DDSAlphaPixels : 0);
				Header.PixelFormat.RGBBitCount = FormatInfo.BitsPerPixel;

				// here we assume RGB8 or RGBA8
				switch (Img.Format)
				{
				case TextureFormat::RGB8:
					Header.PixelFormat.RBitMask = 0x0000FF;
					Header.PixelFormat.GBitMask = 0x00FF00;
					Header.PixelFormat.BBitMask = 0xFF0000;
					Header.PixelFormat.ABitMask = 0;
					break;
				case TextureFormat::RGBA8:
					Header.PixelFormat.RBitMask = 0x000000FF;
					Header.PixelFormat.GBitMask = 0x0000FF00;
					Header.PixelFormat.BBitMask = 0x00FF0000;
					Header.PixelFormat.ABitMask = 0xFF000000;
					break;
				case TextureFormat::RGBA16F:
					Header.PixelFormat.Flags = DDSFourCC | DDSAlphaPixels;
					Header.PixelFormat.RGBBitCount = 0;
					Header.PixelFormat.RBitMask = 0;
					Header.PixelFormat.GBitMask = 0;
					Header.PixelFormat.BBitMask = 0;
					Header.PixelFormat.ABitMask = 0;
					Header.PixelFormat.FourCC = DDSFourccARGB16F;
					break;
				case TextureFormat::RGBA32F:
					Header.PixelFormat.Flags = DDSFourCC | DDSAlphaPixels;
					Header.PixelFormat.RGBBitCount = 0;
					Header.PixelFormat.RBitMask = 0;
					Header.PixelFormat.GBitMask = 0;
					Header.PixelFormat.BBitMask = 0;
					Header.PixelFormat.ABitMask = 0;
					Header.PixelFormat.FourCC = DDSFourccARGB32F;
					break;
				default:
					Log::Error("[ImageSaveDDS] Unsupported uncompressed format, only RGB8, RGBA8, RGBA16F and RGBA32F: %s", FormatInfo.FriendlyName);
					DEBUGBREAK();
					return false;
					break;
				}
			}
			
			Header.Caps1  = DDSTexture;
			Header.Caps1 |= (Img.MipMaps > 1 ? (DDSMipMap | DDSComplex) : 0);
			Header.Caps1 |= (Img.Type == ImageType::ImageCube ? DDSComplex : 0);
			Header.Caps2 = Img.Type == ImageType::Image3D ? DDSVolume : 0;
			Header.Caps2 |= (Img.Type == ImageType::ImageCube ? (DDSCubemap | DDSCubemapAllFaces) : 0);
			Header.Caps3 = 0;
			Header.Caps4 = 0;
			Header.Reserved2 = 0;
		}

		if (HasDX10Header)
		{
			Header.PixelFormat.FourCC = DDS_FOURCC("DX10");

			// find dxgi format
			switch (Img.Format)
			{
			case TextureFormat::BC6H: Header10.Format = 95; break; // DXGI_FORMAT_BC6H_UF16
			case TextureFormat::BC7:  Header10.Format = 98; break; // DXGI_FORMAT_BC7_UNORM
			default:
			{
				for (int i = 0; i < sizeofarray(DdsDxgiFormat_Map); i++)
				{
					if (Img.Format == DdsDxgiFormat_Map[i])
					{
						Header10.Format = i;
						break;
					}
				}
				break;
			}
			}
			
			Header10.ResourceDimension = Img.Type == ImageType::Image3D ? DDSDimensionTexture3D : DDSDimensionTexture2D;
			Header10.MiscFlag = Img.Type == ImageType::ImageCube ? DDS_DX10_Misc_Cube : 0;
			Header10.ArraySize = 1; // no support for arrays in the engine
			Header10.MiscFlags2 = DDS_DX10_Misc2_AlphaUnkown;
		} else
		{
			switch (Img.Format)
			{
			case TextureFormat::DXT1: Header.PixelFormat.FourCC = DDS_FOURCC("DXT1"); break;
			case TextureFormat::DXT3: Header.PixelFormat.FourCC = DDS_FOURCC("DXT3"); break;
			case TextureFormat::DXT5: Header.PixelFormat.FourCC = DDS_FOURCC("DXT5"); break;
			default:                  Header.PixelFormat.FourCC = 0; break;
			}
		}

		Stream.Write(Header);
		if (HasDX10Header)
		{
			Stream.Write(Header10);
		}

		u64 DataSize = ImageUtils::ImageCalcByteSize(Img.Width, Img.Height, Img.Depth, Img.MipMaps, Img.Format);
		if (Img.Type == ImageType::ImageCube)
			DataSize *= 6;

		bool result = Stream.WriteBytes(Img.Data, DataSize);

		return true;

	}
}
