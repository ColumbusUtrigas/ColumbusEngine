#pragma once

#include <Common/Image/Image.h>
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

		bool operator==(const SamplerDesc&) const = default;
	};

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
