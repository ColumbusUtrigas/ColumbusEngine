#pragma once

#include <Common/Image/Image.h>
#include <algorithm>
#include <cstring>
#include <memory>

#include <Core/Blob.h>
#include <Core/Reflection.h>

namespace Columbus
{

	enum class TextureFilter2
	{
		Nearest,
		Linear,
	};

	enum class TextureAddressMode
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		// ClampToBorder,
	};

	enum class ColourSpaceMode
	{
		Linear,
		SRGB,
	};

	enum class CompressionMode
	{
		None,
		BC1,
		BC3,
		BC5,
		BC6H,
		BC7,
	};

	enum class MipGenMode
	{
		None,
		Default,
		NormalMap,
	};

	enum class TextureType
	{
		Texture1D,
		Texture2D,
		Texture3D,
		TextureCube,
		Texture1DArray,
		Texture2DArray,
		TextureCubeArray,
	};

	enum class TextureUsage
	{
		Sampled = 1,
		Storage = 2,
		RenderTargetColor = 4,
		RenderTargetDepth = 8,

		StorageSampled = Sampled | Storage,
	};
	IMPLEMENT_ENUM_CLASS_BITOPS(TextureUsage);

	static const char* TextureTypeToString(TextureType Type)
	{
		switch (Type)
		{
		case TextureType::Texture1D:        return "Texture1D";
		case TextureType::Texture2D:        return "Texture2D";
		case TextureType::Texture3D:        return "Texture3D";
		case TextureType::TextureCube:      return "TextureCube";
		case TextureType::Texture1DArray:   return "Texture1DArray";
		case TextureType::Texture2DArray:   return "Texture2DArray";
		case TextureType::TextureCubeArray: return "TextureCubeArray";
		default:                            return "Undefined";
		}
	}

	static const char* TextureUsageToString(TextureUsage Usage)
	{
		switch (Usage)
		{
		case TextureUsage::Sampled:           return "Sampled";
		case TextureUsage::Storage:           return "Storage";
		case TextureUsage::StorageSampled:    return "StorageSampled";
		case TextureUsage::RenderTargetColor: return "RenderTargetColor";
		case TextureUsage::RenderTargetDepth: return "RenderTargetDepth";
		default:                              return "Undefined";
		}
	}

	struct SamplerDesc
	{
		u32 Anisotropy = 1;
		float MinLOD = 0.0f, MaxLOD = 1000.0f;
		float LodBias = 0.0f;
		TextureAddressMode AddressU = TextureAddressMode::Repeat;
		TextureAddressMode AddressV = TextureAddressMode::Repeat;
		TextureAddressMode AddressW = TextureAddressMode::Repeat;
		TextureFilter2 MinFilter = TextureFilter2::Linear;
		TextureFilter2 MagFilter = TextureFilter2::Linear;
		TextureFilter2 MipFilter = TextureFilter2::Linear;

		static SamplerDesc Create(TextureFilter2 Filter = TextureFilter2::Linear, TextureAddressMode Address = TextureAddressMode::ClampToEdge);

		template <TextureFilter2 Filter = TextureFilter2::Linear, TextureAddressMode Address = TextureAddressMode::ClampToEdge>
		static SamplerDesc Create()
		{
			return Create(Filter, Address);
		}

		bool operator==(const SamplerDesc&) const = default;
	};

	inline SamplerDesc SamplerDesc::Create(TextureFilter2 Filter, TextureAddressMode Address)
	{
		SamplerDesc Desc;
		Desc.AddressU = Address;
		Desc.AddressV = Address;
		Desc.AddressW = Address;
		Desc.MagFilter = Filter;
		Desc.MinFilter = Filter;
		Desc.MipFilter = Filter;
		return Desc;
	}

	struct HashSamplerDesc
	{
		size_t operator()(const SamplerDesc&) const;
	};

	struct TextureDesc2
	{
		// Image description
		TextureType Type = TextureType::Texture2D;
		TextureUsage Usage = TextureUsage::Sampled;
		uint32 Width = 1, Height = 1, Depth = 1;
		uint32 Mips = 1, ArrayLayers = 1;
		uint32 Samples = 1; // MSAA samples count
		TextureFormat Format {};

		// Sampler description
		uint32 Anisotropy = 1;
		float MinLOD = 0.0f, MaxLOD = 1000.0f;
		TextureAddressMode AddressU = TextureAddressMode::Repeat;
		TextureAddressMode AddressV = TextureAddressMode::Repeat;
		TextureAddressMode AddressW = TextureAddressMode::Repeat;
		TextureFilter2 MinFilter = TextureFilter2::Linear;
		TextureFilter2 MagFilter = TextureFilter2::Linear;
		TextureFilter2 MipFilter = TextureFilter2::Linear;

		bool operator==(const TextureDesc2&) const = default;
	};

	struct HashTextureDesc2
	{
		size_t operator()(const TextureDesc2&) const;
	};

	// describes a view of a texture subresource range (see DeviceVulkan::CreateTextureView)
	struct TextureViewDesc
	{
		TextureType ViewType = TextureType::Texture2D;
		u32 BaseMip = 0;
		u32 MipCount = 0; // 0 = all remaining mips
		u32 BaseLayer = 0;
		u32 LayerCount = 0; // 0 = all remaining layers
	};

	struct TextureImportSettings
	{
		ColourSpaceMode ColourSpace = ColourSpaceMode::SRGB;
		CompressionMode Compression = CompressionMode::None;
		MipGenMode MipGen = MipGenMode::Default;
		TextureAddressMode AddressMode = TextureAddressMode::Repeat;
		int MaxSize = 0;
	};

	struct TextureStoredPixels
	{
		int Width = 0;
		int Height = 0;
		int Depth = 1;
		int Mips = 1;
		std::string Format;
		std::string Type;
		Blob Pixels;
	};

	namespace TextureUtils
	{
		inline TextureType TextureTypeFromImageType(ImageType Type)
		{
			switch (Type)
			{
			case ImageType::Image3D:      return TextureType::Texture3D;
			case ImageType::ImageCube:    return TextureType::TextureCube;
			case ImageType::Image2DArray: return TextureType::Texture2DArray;
			case ImageType::Image2D:
			default:                      return TextureType::Texture2D;
			}
		}

		inline ImageType ImageTypeFromTextureType(TextureType Type)
		{
			switch (Type)
			{
			case TextureType::Texture3D:        return ImageType::Image3D;
			case TextureType::TextureCube:
			case TextureType::TextureCubeArray: return ImageType::ImageCube;
			case TextureType::Texture2DArray:   return ImageType::Image2DArray;
			case TextureType::Texture1D:
			case TextureType::Texture2D:
			case TextureType::Texture1DArray:
			default:                            return ImageType::Image2D;
			}
		}

		inline u32 TextureLayerCount(TextureType Type, u32 ArrayLayers)
		{
			if (Type == TextureType::TextureCube)
				return 6u;

			if (Type == TextureType::TextureCubeArray ||
				Type == TextureType::Texture1DArray ||
				Type == TextureType::Texture2DArray)
				return std::max(1u, ArrayLayers);

			return 1u;
		}

		inline u32 TextureLayerCount(const TextureDesc2& Desc)
		{
			return TextureLayerCount(Desc.Type, Desc.ArrayLayers);
		}

		inline TextureDesc2 NormalizeTextureDesc(TextureDesc2 Desc)
		{
			Desc.Width = std::max(1u, Desc.Width);
			Desc.Height = std::max(1u, Desc.Height);
			Desc.Depth = Desc.Type == TextureType::Texture3D ? std::max(1u, Desc.Depth) : 1u;
			Desc.Mips = std::max(1u, Desc.Mips);
			Desc.ArrayLayers = TextureLayerCount(Desc);
			Desc.Samples = std::max(1u, Desc.Samples);
			return Desc;
		}

		inline u32 TextureArrayLayersForImage(const Image& ImageData, u32 FallbackArrayLayers = 1)
		{
			return ImageData.GetType() == ImageType::ImageCube ? ImageUtils::ImageLayerCount(ImageData.GetType()) : std::max(1u, FallbackArrayLayers);
		}

		inline void SetTextureDescImageFields(TextureDesc2& Desc, const Image& ImageData, u32 FallbackArrayLayers = 1)
		{
			Desc.Type = TextureTypeFromImageType(ImageData.GetType());
			Desc.Width = std::max(1u, ImageData.GetWidth());
			Desc.Height = std::max(1u, ImageData.GetHeight());
			Desc.Depth = ImageData.GetType() == ImageType::Image3D ? std::max(1u, ImageData.GetDepth()) : 1u;
			Desc.Mips = std::max(1u, ImageData.GetMipmapsCount());
			Desc.ArrayLayers = TextureArrayLayersForImage(ImageData, FallbackArrayLayers);
			Desc.Samples = 1;
			Desc.Format = ImageData.GetFormat();
		}

		inline void SetBorrowedImageViewFromStoredPixels(Image& ImageView, const TextureStoredPixels& Stored)
		{
			ImageView.Width = (u32)Stored.Width;
			ImageView.Height = (u32)Stored.Height;
			ImageView.Depth = (u32)Stored.Depth;
			ImageView.MipMaps = (u32)Stored.Mips;
			ImageView.Format = TextureFormatFromFriendlyName(Stored.Format);
			ImageView.Type = ImageTypeFromString(Stored.Type);
			ImageView.Exist = !Stored.Pixels.Empty();
			ImageView.Size = Stored.Pixels.Size();
			ImageView.Data = const_cast<u8*>(Stored.Pixels.Data());
		}

		inline void StoreImagePixels(TextureStoredPixels& Stored, const Image& ImageData)
		{
			Stored.Width = (int)ImageData.Width;
			Stored.Height = (int)ImageData.Height;
			Stored.Depth = (int)std::max(1u, ImageData.Depth);
			Stored.Mips = (int)std::max(1u, ImageData.MipMaps);
			Stored.Format = TextureFormatGetInfo(ImageData.Format).FriendlyName;
			Stored.Type = ImageTypeToString(ImageData.Type);
			Stored.Pixels.Bytes.resize((size_t)ImageData.GetFullSize());
			if (!Stored.Pixels.Bytes.empty())
			{
				memcpy(Stored.Pixels.Data(), ImageData.Data, Stored.Pixels.Size());
			}
		}

		inline u64 ComputeStoredPixelByteSize(const TextureStoredPixels& Stored)
		{
			if (Stored.Width <= 0 || Stored.Height <= 0 || Stored.Depth <= 0 || Stored.Mips <= 0)
				return 0;

			const TextureFormat Format = TextureFormatFromFriendlyName(Stored.Format);
			if (Format == TextureFormat::Unknown)
				return 0;

			const ImageType Type = ImageTypeFromString(Stored.Type);
			return ImageUtils::ImageCalcByteSize((u32)Stored.Width, (u32)Stored.Height, (u32)Stored.Depth, (u32)Stored.Mips, Format, Type);
		}

		inline bool ValidateStoredPixelByteSize(const TextureStoredPixels& Stored, u64* OutExpectedBytes = nullptr)
		{
			const u64 ExpectedBytes = ComputeStoredPixelByteSize(Stored);
			if (OutExpectedBytes != nullptr)
			{
				*OutExpectedBytes = ExpectedBytes;
			}
			return Stored.Pixels.Size() == ExpectedBytes;
		}
	}

	class Texture2
	{
	private:
		TextureDesc2 _Desc{};
	protected:
		Texture2(const TextureDesc2& Desc) : _Desc(Desc) {}

		u64 Size = 0;
	public:
		Texture2() = default;

		std::string SourcePath;
		TextureImportSettings ImportSettings;
		TextureStoredPixels Source;
		TextureStoredPixels Cooked;

		const TextureDesc2& GetDesc() const { return _Desc; }
		u64 GetSize() const { return Size; }
		void SetDesc(const TextureDesc2& Desc) { _Desc = Desc; }

		void CopyAssetFieldsFrom(const Texture2& Other)
		{
			SourcePath = Other.SourcePath;
			ImportSettings = Other.ImportSettings;
			Source = Other.Source;
			Cooked = Other.Cooked;
		}

		virtual ~Texture2() = default;
	};

	class Sampler
	{
	private:
		SamplerDesc _Desc;
	protected:
		Sampler(const SamplerDesc& Desc) : _Desc(Desc) {}
	public:
		const SamplerDesc& GetDesc() const { return _Desc; }
	};

}

CREFLECT_DECLARE_ENUM(Columbus::ColourSpaceMode, "447C6FA6-6137-4E4C-BF10-4BAA90B60F7D");
CREFLECT_DECLARE_ENUM(Columbus::CompressionMode, "6D0EBE89-2F60-4AA6-9DB5-A16B71B0E1B7");
CREFLECT_DECLARE_ENUM(Columbus::MipGenMode, "0E59FBDB-98FE-4268-A08B-9F6B0C8A3B65");
CREFLECT_DECLARE_ENUM(Columbus::TextureAddressMode, "239A1AF8-0D8A-47F2-8BA1-B30DD073181D");
CREFLECT_DECLARE_STRUCT(Columbus::TextureImportSettings, 1, "D34242E8-A478-45C5-AF28-0D66C46D9C32");
CREFLECT_DECLARE_STRUCT(Columbus::TextureStoredPixels, 1, "8E3522BF-4A46-4D60-B8D6-9376801B8C11");
CREFLECT_DECLARE_STRUCT(Columbus::Texture2, 3, "1B4AF05B-674A-4B68-8C72-1B46644DA0EC");
