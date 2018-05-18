#include <Common/Image/Image.h>
#include <System/File.h>
#include <Core/Core.h>

namespace Columbus
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
	constexpr uint32 DDSFourCC_DXT1 = 0x31545844; // "DXT1"
	constexpr uint32 DDSFourCC_DXT2 = 0x32545844; // "DXT2"
	constexpr uint32 DDSFourCC_DXT3 = 0x33545844; // "DXT3"
	constexpr uint32 DDSFourCC_DXT4 = 0x34545844; // "DXT4"
	constexpr uint32 DDSFourCC_DXT5 = 0x35545844; // "DXT5"

	/*
	* DDS Image pixel format struct
	* @see: https://msdn.microsoft.com/en-us/library/windows/desktop/bb943984(v=vs.85).aspx
	*/
	typedef struct
	{
		uint32 Size;
		uint32 Flags;
		uint32 FourCC;
		uint32 RGBBitmask;
		uint32 RBitMask;
		uint32 GBitMask;
		uint32 BBitMask;
		uint32 ABitMask;
	} DDS_PIXELFORMAT;

	/*
	* DDS Image header struct
	* @see: https://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx
	*/
	typedef struct
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
	} DDS_HEADER;
	/*
	* DDS Image DXGI format enumeration
	* @see: https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
	*/
	typedef enum
	{
		DXGI_FORMAT_UNKNOWN = 0,
		DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
		DXGI_FORMAT_R32G32B32A32_UINT = 3,
		DXGI_FORMAT_R32G32B32A32_SINT = 4,
		DXGI_FORMAT_R32G32B32_TYPELESS = 5,
		DXGI_FORMAT_R32G32B32_FLOAT = 6,
		DXGI_FORMAT_R32G32B32_UINT = 7,
		DXGI_FORMAT_R32G32B32_SINT = 8,
		DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
		DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
		DXGI_FORMAT_R16G16B16A16_UNORM = 11,
		DXGI_FORMAT_R16G16B16A16_UINT = 12,
		DXGI_FORMAT_R16G16B16A16_SNORM = 13,
		DXGI_FORMAT_R16G16B16A16_SINT = 14,
		DXGI_FORMAT_R32G32_TYPELESS = 15,
		DXGI_FORMAT_R32G32_FLOAT = 16,
		DXGI_FORMAT_R32G32_UINT = 17,
		DXGI_FORMAT_R32G32_SINT = 18,
		DXGI_FORMAT_R32G8X24_TYPELESS = 19,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
		DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
		DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
		DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
		DXGI_FORMAT_R10G10B10A2_UNORM = 24,
		DXGI_FORMAT_R10G10B10A2_UINT = 25,
		DXGI_FORMAT_R11G11B10_FLOAT = 26,
		DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
		DXGI_FORMAT_R8G8B8A8_UNORM = 28,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
		DXGI_FORMAT_R8G8B8A8_UINT = 30,
		DXGI_FORMAT_R8G8B8A8_SNORM = 31,
		DXGI_FORMAT_R8G8B8A8_SINT = 32,
		DXGI_FORMAT_R16G16_TYPELESS = 33,
		DXGI_FORMAT_R16G16_FLOAT = 34,
		DXGI_FORMAT_R16G16_UNORM = 35,
		DXGI_FORMAT_R16G16_UINT = 36,
		DXGI_FORMAT_R16G16_SNORM = 37,
		DXGI_FORMAT_R16G16_SINT = 38,
		DXGI_FORMAT_R32_TYPELESS = 39,
		DXGI_FORMAT_D32_FLOAT = 40,
		DXGI_FORMAT_R32_FLOAT = 41,
		DXGI_FORMAT_R32_UINT = 42,
		DXGI_FORMAT_R32_SINT = 43,
		DXGI_FORMAT_R24G8_TYPELESS = 44,
		DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
		DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
		DXGI_FORMAT_R8G8_TYPELESS = 48,
		DXGI_FORMAT_R8G8_UNORM = 49,
		DXGI_FORMAT_R8G8_UINT = 50,
		DXGI_FORMAT_R8G8_SNORM = 51,
		DXGI_FORMAT_R8G8_SINT = 52,
		DXGI_FORMAT_R16_TYPELESS = 53,
		DXGI_FORMAT_R16_FLOAT = 54,
		DXGI_FORMAT_D16_UNORM = 55,
		DXGI_FORMAT_R16_UNORM = 56,
		DXGI_FORMAT_R16_UINT = 57,
		DXGI_FORMAT_R16_SNORM = 58,
		DXGI_FORMAT_R16_SINT = 59,
		DXGI_FORMAT_R8_TYPELESS = 60,
		DXGI_FORMAT_R8_UNORM = 61,
		DXGI_FORMAT_R8_UINT = 62,
		DXGI_FORMAT_R8_SNORM = 63,
		DXGI_FORMAT_R8_SINT = 64,
		DXGI_FORMAT_A8_UNORM = 65,
		DXGI_FORMAT_R1_UNORM = 66,
		DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
		DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
		DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
		DXGI_FORMAT_BC1_TYPELESS = 70,
		DXGI_FORMAT_BC1_UNORM = 71,
		DXGI_FORMAT_BC1_UNORM_SRGB = 72,
		DXGI_FORMAT_BC2_TYPELESS = 73,
		DXGI_FORMAT_BC2_UNORM = 74,
		DXGI_FORMAT_BC2_UNORM_SRGB = 75,
		DXGI_FORMAT_BC3_TYPELESS = 76,
		DXGI_FORMAT_BC3_UNORM = 77,
		DXGI_FORMAT_BC3_UNORM_SRGB = 78,
		DXGI_FORMAT_BC4_TYPELESS = 79,
		DXGI_FORMAT_BC4_UNORM = 80,
		DXGI_FORMAT_BC4_SNORM = 81,
		DXGI_FORMAT_BC5_TYPELESS = 82,
		DXGI_FORMAT_BC5_UNORM = 83,
		DXGI_FORMAT_BC5_SNORM = 84,
		DXGI_FORMAT_B5G6R5_UNORM = 85,
		DXGI_FORMAT_B5G5R5A1_UNORM = 86,
		DXGI_FORMAT_B8G8R8A8_UNORM = 87,
		DXGI_FORMAT_B8G8R8X8_UNORM = 88,
		DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
		DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
		DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
		DXGI_FORMAT_BC6H_TYPELESS = 94,
		DXGI_FORMAT_BC6H_UF16 = 95,
		DXGI_FORMAT_BC6H_SF16 = 96,
		DXGI_FORMAT_BC7_TYPELESS = 97,
		DXGI_FORMAT_BC7_UNORM = 98,
		DXGI_FORMAT_BC7_UNORM_SRGB = 99,
		DXGI_FORMAT_AYUV = 100,
		DXGI_FORMAT_Y410 = 101,
		DXGI_FORMAT_Y416 = 102,
		DXGI_FORMAT_NV12 = 103,
		DXGI_FORMAT_P010 = 104,
		DXGI_FORMAT_P016 = 105,
		DXGI_FORMAT_420_OPAQUE = 106,
		DXGI_FORMAT_YUY2 = 107,
		DXGI_FORMAT_Y210 = 108,
		DXGI_FORMAT_Y216 = 109,
		DXGI_FORMAT_NV11 = 110,
		DXGI_FORMAT_AI44 = 111,
		DXGI_FORMAT_IA44 = 112,
		DXGI_FORMAT_P8 = 113,
		DXGI_FORMAT_A8P8 = 114,
		DXGI_FORMAT_B4G4R4A4_UNORM = 115,
		DXGI_FORMAT_P208 = 130,
		DXGI_FORMAT_V208 = 131,
		DXGI_FORMAT_V408 = 132,
		DXGI_FORMAT_FORCE_UINT = 0xffffffff
	} DXGI_FORMAT;
	/*
	* DDS Image Resource dimension enumeration
	* @see: https://msdn.microsoft.com/en-us/library/windows/desktop/bb172411(v=vs.85).aspx
	*/
	typedef enum
	{
		D3D10_RESOURCE_DIMENSION_UNKNOWN = 0,
		D3D10_RESOURCE_DIMENSION_BUFFER = 1,
		D3D10_RESOURCE_DIMENSION_TEXTURE1D = 2,
		D3D10_RESOURCE_DIMENSION_TEXTURE2D = 3,
		D3D10_RESOURCE_DIMENSION_TEXTURE3D = 4
	} D3D10_RESOURCE_DIMENSION;
	/*
	* DDS Image DX10 header struct
	* @see: https://msdn.microsoft.com/en-us/library/windows/desktop/bb943983(v=vs.85).aspx
	*/
	typedef struct
	{
		DXGI_FORMAT Format;
		D3D10_RESOURCE_DIMENSION ResourceDimension;
		uint32 MiscFlag;
		uint32 ArraySize;
		uint32 MiscFlags2;
	} DDS_HEADER_DX10;

	typedef struct
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
	} DDS_HEADER_FLAGS;

	static TextureFormat FourCCDecode(uint32 FourCC)
	{
		TextureFormat Result = TextureFormat::Unknown;

		switch (FourCC)
		{
		case DDSFourCC_DXT1: Result = TextureFormat::S3TC_A1; break;
		case DDSFourCC_DXT2:
		case DDSFourCC_DXT3: Result = TextureFormat::S3TC_A4; break;
		case DDSFourCC_DXT4:
		case DDSFourCC_DXT5: Result = TextureFormat::S3TC_A8; break;
		default: Result = TextureFormat::Unknown; break;
		}

		return Result;
	}

	bool ImageIsDDS(std::string FileName)
	{
		File DDSImageFile(FileName, "rb");
		if (!DDSImageFile.IsOpened()) return false;

		uint8 Magic[4];
		DDSImageFile.ReadBytes(Magic, sizeof(Magic));
		DDSImageFile.Close();

		if (Memory::Memcmp(Magic, "DDS ", 4) == 0)
		{
			return true;
		}

		return false;
	}

	bool ImageIsDDSMemory(const uint8* Data, uint64 Size)
	{
		if (Data == nullptr || Size == 0)
		{
			return false;
		}

		if (Memory::Memcmp(Data, "DDS ", 4) == 0)
		{
			return true;
		}

		return false;
	}

	uint8* ImageLoadDDS(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint64& OutSize, uint32& OutMipMaps, TextureFormat& OutFormat)
	{
		File DDSImageFile(FileName, "rb");
		if (!DDSImageFile.IsOpened())
		{
			Log::error("ImageLoadDDS() error: Couldn't load DDS: Couldn'y open file");
			return nullptr;
		}

		uint8* Data = new uint8[DDSImageFile.GetSize()];
		DDSImageFile.Read(Data, DDSImageFile.GetSize(), 1);
		uint8* Result = ImageLoadDDSMemory(Data, DDSImageFile.GetSize(), OutWidth, OutHeight, OutSize, OutMipMaps, OutFormat);
		DDSImageFile.Close();

		delete[] Data;
		return Result;
	}

	uint8* ImageLoadDDSMemory(const uint8* Data, uint64 Size, uint32& OutWidth, uint32& OutHeight, uint64& OutSize, uint32& OutMipMaps, TextureFormat& OutFormat)
	{
		if (Data == nullptr || Size == 0)
		{
			return nullptr;
		}

		if (Size < sizeof(DDS_HEADER))
		{
			Log::error("ImageLoadDDSMemory() error: Couldn't load DDS: DDS size less than header size");
			return nullptr;
		}

		DDS_HEADER Header;
		DDS_HEADER_FLAGS HeaderFlags;
		Header = *(DDS_HEADER*)(Data); Data += sizeof(DDS_HEADER);
		HeaderFlags.Read(Header.Flags);

		if (HeaderFlags.Caps == false ||
		    HeaderFlags.Height == false ||
		    HeaderFlags.Width == false ||
		    HeaderFlags.PixelFormat == false ||
		    HeaderFlags.Texture == false)
		{
			Log::error("ImageLoadDDSMemory() error: Couldn't load DDS: Invalid DDS flags");
			return nullptr;
		}


		if (Header.PixelFormat.FourCC == ('D' | ('X' << 8) | ('1' << 16) | ('0' << 24)))
		{
			Log::error("ImageLoadDDSMemory() error: Couldn't load DDS: It is a DX10 DDS texture");
			return nullptr;
		}

		if ((Header.PixelFormat.Flags & 0x4) == 0)
		{
			Log::error("ImageLoadDDSMemory() error: Couldn't load DDS: Invalid DDS Pixel format flags");
			return nullptr;
		}

		OutFormat = FourCCDecode(Header.PixelFormat.FourCC);
		OutWidth = Header.Width;
		OutHeight = Header.Height;
		OutMipMaps = Header.MipMapCount;

		uint32 BlockSize = 0;

		if (OutFormat == TextureFormat::S3TC_A1)
		{
			BlockSize = 8;
		}
		else
		{
			BlockSize = 16;
		}

		OutSize = ((Header.Width + 3) / 4) * ((Header.Height + 3) / 4) * BlockSize;

		uint8* Buffer = new uint8[OutSize];
		std::copy(Data, Data + OutSize, Buffer);

		return Buffer;
	}

}









