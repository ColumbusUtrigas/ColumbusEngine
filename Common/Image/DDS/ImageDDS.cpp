#include <Common/Image/Image.h>
#include <Common/Image/DDS/ImageDDS.h>
#include <System/File.h>
#include <System/Log.h>
#include <Core/Assert.h>
#include <cstring>

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

	static TextureFormat FourCCDecode(uint32 FourCC)
	{
		TextureFormat Result = TextureFormat::Unknown;

		switch (FourCC)
		{
		case DDSFourCC_DXT1: Result = TextureFormat::DXT1; break;
		case DDSFourCC_DXT2:
		case DDSFourCC_DXT3: Result = TextureFormat::DXT3; break;
		case DDSFourCC_DXT4:
		case DDSFourCC_DXT5: Result = TextureFormat::DXT5; break;
		default: Result = TextureFormat::Unknown; break;
		}

		return Result;
	}

	static uint8* ImageLoadDDSMemory(const uint8* Data, uint64 Size, uint32& OutWidth, uint32& OutHeight, uint32& OutMipMaps, ImageLoader::Type& OutImageType, TextureFormat& OutFormat)
	{
		if (Data == nullptr || Size == 0)
		{
			return nullptr;
		}

		if (Size < sizeof(DDS_HEADER))
		{
			Log::Error("ImageLoadDDSMemory() error: Couldn't load DDS: DDS size less than header size");
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
			Log::Error("ImageLoadDDSMemory() error: Couldn't load DDS: Invalid DDS flags");
			return nullptr;
		}


		if (Header.PixelFormat.FourCC == ('D' | ('X' << 8) | ('1' << 16) | ('0' << 24)))
		{
			Log::Error("ImageLoadDDSMemory() error: Couldn't load DDS: It is a DX10 DDS texture");
			return nullptr;
		}

		if ((Header.PixelFormat.Flags & DDSFourCC) == 0x00 &&
		    (Header.PixelFormat.Flags & DDSRGBA)   == 0x00 &&
		    (Header.PixelFormat.Flags & DDSRGB)    == 0x00)
		{
			Log::Error("ImageLoadDDSMemory() error: Couldn't load DDS: Invalid DDS Pixel format flags");
			return nullptr;
		}

		if (Header.Caps2 & DDSCubemap) //Check for cubemap
		{
			if ((Header.Caps2 & DDSCubemapPositiveX) == 0 ||  //Check cubemap for complexity
			    (Header.Caps2 & DDSCubemapNegativeX) == 0 ||
			    (Header.Caps2 & DDSCubemapPositiveY) == 0 ||
			    (Header.Caps2 & DDSCubemapNegativeY) == 0 ||
			    (Header.Caps2 & DDSCubemapPositiveZ) == 0 ||
			    (Header.Caps2 & DDSCubemapNegativeZ) == 0 ||
			    (Header.Caps2 & DDSCubemapAllFaces)  == 0)
			{
				Log::Error("ImageLoadDDSMemory() error: Couldn't load DDS: cubemap is not complex");
				return nullptr;
			}

			OutImageType = ImageLoader::Type::ImageCube;
		}

		uint64 DataSize = 0;

		if (Header.PixelFormat.Flags & DDSFourCC)
		{
			OutFormat = FourCCDecode(Header.PixelFormat.FourCC);

			uint32 BlockSize = GetBlockSizeFromFormat(OutFormat);

			if ((Header.Caps1 & DDSMipMap) != 0)
			{
				for (uint32 i = 0; i < Header.MipMapCount; i++)
				{
					DataSize += (((Header.Width >> i) + 3) / 4) * (((Header.Height >> i) + 3) / 4) * BlockSize;
				}
			}
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

			uint32 BPP = GetBPPFromFormat(OutFormat);

			if ((Header.Caps1 & DDSMipMap) != 0)
			{
				for (uint32 i = 0; i < Header.MipMapCount; i++)
				{
					DataSize += (Header.Width >> i) * (Header.Height >> i) * BPP;
				}
			}
		}

		OutWidth = Header.Width;
		OutHeight = Header.Height;

		if ((Header.Caps1 & DDSMipMap) != 0)
			OutMipMaps = Header.MipMapCount;
		else
			OutMipMaps = 1;

		if (OutImageType == ImageLoader::Type::ImageCube) DataSize *= 6;

		uint8* Buffer = new uint8[DataSize];
		memcpy(Buffer, Data, DataSize);

		return Buffer;
	}

	bool ImageLoaderDDS::IsDDS(const char* FileName)
	{
		File DDSImageFile(FileName, "rb");
		if (!DDSImageFile.IsOpened()) return false;

		uint8 Magic[4];
		DDSImageFile.ReadBytes(Magic, sizeof(Magic));
		DDSImageFile.Close();

		if (memcmp(Magic, "DDS ", 4) == 0)
		{
			return true;
		}

		return false;
	}

	bool ImageLoaderDDS::Load(const char* FileName)
	{
		File DDSImageFile(FileName, "rb");
		if (!DDSImageFile.IsOpened())
		{
			Log::Error("ImageLoadDDS() error: Couldn't load DDS: Couldn'y open file");
			return false;
		}

		ImageType = Type::Image2D;

		uint8* TmpData = new uint8[DDSImageFile.GetSize()];
		DDSImageFile.Read(TmpData, DDSImageFile.GetSize(), 1);
		Data = ImageLoadDDSMemory(TmpData, DDSImageFile.GetSize(), Width, Height, Mipmaps, ImageType, Format);
		DDSImageFile.Close();

		delete[] TmpData;
		return (Data != nullptr);
	}

}


