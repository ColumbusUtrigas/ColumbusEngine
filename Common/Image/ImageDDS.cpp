#include <Common/Image/Image.h>
#include <System/File.h>
#include <System/Log.h>
#include <Core/Assert.h>
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
		case DDS_FOURCC("DXT3"): return TextureFormat::DXT3; // BC2_UNORM
		case DDS_FOURCC("DXT4"): // BC3_UNORM
		case DDS_FOURCC("DXT5"): return TextureFormat::DXT5; // BC3_UNORM
		case DDS_FOURCC("ATI2"): return TextureFormat::DXT5; // BC5_UNORM
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

	bool ImageLoadFromStreamDDS(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData)
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
				Log::Error("ImageLoadFromStreamDDS() error: 3D textures are not supported");
				return false;
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
				Log::Error("ImageLoadFromStreamDDS() error: 3D textures are not supported");
				return false;
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

		u32 Depth = 1; // TODO: 3D textures
		u64 DataSize = ImageUtils::ImageCalcByteSize(OutWidth, OutHeight, Depth, OutMips, OutFormat);

		if (OutType == ImageType::ImageCube)
			DataSize *= 6;

		OutData = new u8[DataSize];
		Stream.ReadBytes(OutData, DataSize);

		return true;
	}

}
