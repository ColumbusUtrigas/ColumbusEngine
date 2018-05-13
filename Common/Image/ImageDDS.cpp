#include <Common/Image/Image.h>
#include <System/File.h>
#include <Core/Core.h>

namespace Columbus
{

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
		uint32 MipMapcount;
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
			Caps = Flags & 0x1;
			Height = Flags & 0x2;
			Width = Flags & 0x4;
			Pitch = Flags & 0x8;
			PixelFormat = Flags & 0x1000;
			MipMapCount = Flags & 0x20000;
			LinearSize = Flags & 0x80000;
			Depth = Flags & 0x800000;

			Texture = (Flags & 0x1) | (Flags & 0x2) | (Flags & 0x4) | (Flags & 0x1000);
			Mipmap = Flags & 0x20000;
			Volume = Flags & 0x800000;
		}
	} DDS_HEADER_FLAGS;

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

	uint8* ImageLoadDDS(std::string FileName, uint32& OutWidth, uint32& OutHeight, TextureFormat& OutFormat)
	{
		File DDSImageFile(FileName, "rb");
		if (!DDSImageFile.IsOpened()) return false;

		if (DDSImageFile.GetSize() < sizeof(DDS_HEADER))
		{
			Log::error("ImageLoadDDS() error: Couldn't load DDS file: DDS file size less than header size");
			return nullptr;
		}

		bool HasDX10Header = false;

		DDS_HEADER Header;
		DDS_HEADER_FLAGS HeaderFlags;
		DDS_HEADER_DX10 DX10Header;
		DDSImageFile.Read(&Header, sizeof(Header), 1);
		HeaderFlags.Read(Header.Flags);

		if (HeaderFlags.Caps == false ||
		    HeaderFlags.Height == false ||
		    HeaderFlags.Width == false ||
		    HeaderFlags.PixelFormat == false ||
		    HeaderFlags.Texture == false)
		{
			Log::error("ImageLoadDDS() error: Couldn't load DDS file: Invalid DDS flags");
			return nullptr;
		}


		if (Header.PixelFormat.FourCC == ('D' | ('X' << 8) | ('1' << 16) | ('0' << 24)))
		{
			DDSImageFile.Read(&DX10Header, sizeof(DX10Header), 1);
			HasDX10Header = true;
		}

		if ((Header.PixelFormat.Flags & 0x4) == 0)
		{
			Log::error("ImageLoadDDS() error: Couldn't load DDS file: Invalid DDS Pixel format flags");
			return nullptr;
		}

		switch (Header.PixelFormat.FourCC)
		{
		case 0x31545844u:
			OutFormat = TextureFormat::S3TC_A1; break;
		case 0x32545844u:
		case 0x33545844u:
			OutFormat = TextureFormat::S3TC_A4; break;
		case 0x34545844u:
		case 0x35545844u:
			OutFormat = TextureFormat::S3TC_A8; break;
		default: return nullptr; break;
		}
		

		uint8* Data = new uint8[DDSImageFile.GetSize() - sizeof(DDS_HEADER)];
		DDSImageFile.Read(Data, DDSImageFile.GetSize() - sizeof(DDS_HEADER), 1);
		DDSImageFile.Close();

		OutWidth = Header.Width;
		OutHeight = Header.Height;

		printf("DXT1 Format: %s\n", OutFormat == TextureFormat::S3TC_A1 ? "Yes" : "No");
		printf("Has DX10 Header: %s\n", HasDX10Header ? "Yes" : "No");
		printf("Size: %i\n", Header.Size);
		printf("Width: %i\nHeight: %i\nDepth: %i\n", Header.Width, Header.Height, Header.Depth);

		return Data;
	}

}









