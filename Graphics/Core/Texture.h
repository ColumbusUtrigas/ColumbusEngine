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

	// TODO: REMOVE
	struct TextureDesc
	{
		uint32 Width;
		uint32 Height;
		uint32 LOD;
		uint32 Multisampling;
		TextureFormat Format;

		TextureDesc(uint32 Width, uint32 Height, uint32 LOD, uint32 Multisampling, TextureFormat Format) :
			Width(Width),
			Height(Height),
			LOD(LOD),
			Multisampling(Multisampling),
			Format(Format) {}
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

	// TODO: legacy
	class Texture
	{
	public:
		enum class Filter;
		enum class Anisotropy;
		enum class Wrap;
		enum class Type;
		struct Flags;
		struct Properties;
	protected:
		uint32 Width;
		uint32 Height;

		uint32 MipmapsCount;
		uint32 MipmapLevel;

		uint32 Multisampling = 0;
		TextureFormat Format;
		Type TextureType;
	public:
		enum class Filter
		{
			Point,
			Linear,
			Bilinear,
			Trilinear
		};

		enum class Anisotropy
		{
			Anisotropy1,
			Anisotropy2,
			Anisotropy4,
			Anisotropy8,
			Anisotropy16
		};

		enum class Wrap
		{
			Clamp,
			Repeat,
			MirroredRepeat
		};

		enum class Type
		{
			Texture2D,
			Texture3D,
			TextureCube,
			Texture2DArray
		};

		struct Flags
		{
			Filter Filtering = Filter::Trilinear;
			Anisotropy AnisotropyFilter = Anisotropy::Anisotropy8;
			Wrap Wrapping = Wrap::Repeat;

			Flags() {}
			Flags(Filter Filt, Anisotropy Anis, Wrap Wr) :
				Filtering(Filt),
				AnisotropyFilter(Anis),
				Wrapping(Wr) {}

			bool operator==(const Flags& Other) const
			{
				return Filtering == Other.Filtering &&
				       AnisotropyFilter == Other.AnisotropyFilter &&
				       Wrapping == Other.Wrapping;
			}

			bool operator!=(const Flags& Other) const
			{
				return !(*this == Other);
			}
		};
	public:
		Texture() {}
		Texture(const Texture&) = delete;
		Texture(Texture&&) = delete;

		virtual bool Load(const void* Data, TextureDesc Desc) = 0;
		virtual bool Load(Image& InImage) = 0;
		virtual bool Load(const char* File) = 0;

		virtual bool Create(ImageType InType, TextureDesc Desc) = 0;
		virtual bool Create(Type InType, TextureDesc Desc) = 0;
		virtual bool Create2D(TextureDesc Desc) = 0;
		virtual bool CreateCube(TextureDesc Desc) = 0;

		bool CreateAndLoad(Image& Img, int LOD = 0)
		{
			if (Create(Img.GetType(), TextureDesc(Img.GetWidth(), Img.GetHeight(), LOD, 0, Img.GetFormat())))
			{
				return Load(Img);
			}

			return false;
		}

		virtual void SetFlags(Flags F) = 0;
		Flags GetFlags() const { return TextureFlags; }

		uint32 GetMipmapsCount() const { return MipmapsCount; }
		uint32 GetWidth() const { return Width; }
		uint32 GetHeight() const { return Height; }
		uint32 GetMultisampling() const { return Multisampling; }
		TextureFormat GetFormat() const { return Format; }
		Type GetType() const { return TextureType; }

		virtual void SetMipmapLevel(uint32 Level, uint32 MaxLevel) = 0;
		uint32 GetMipmapLevel() const { return MipmapLevel; }

		virtual void GenerateMipmap() = 0;
		
		virtual ~Texture() {}
	protected:
		Flags TextureFlags;
	};

}

CREFLECT_DECLARE_ENUM(Columbus::ColourSpaceMode, "447C6FA6-6137-4E4C-BF10-4BAA90B60F7D");
CREFLECT_DECLARE_ENUM(Columbus::CompressionMode, "6D0EBE89-2F60-4AA6-9DB5-A16B71B0E1B7");
CREFLECT_DECLARE_ENUM(Columbus::MipGenMode, "0E59FBDB-98FE-4268-A08B-9F6B0C8A3B65");
CREFLECT_DECLARE_ENUM(Columbus::TextureAddressMode, "239A1AF8-0D8A-47F2-8BA1-B30DD073181D");
CREFLECT_DECLARE_STRUCT(Columbus::TextureImportSettings, 1, "D34242E8-A478-45C5-AF28-0D66C46D9C32");
CREFLECT_DECLARE_STRUCT(Columbus::TextureStoredPixels, 1, "8E3522BF-4A46-4D60-B8D6-9376801B8C11");
CREFLECT_DECLARE_STRUCT(Columbus::Texture2, 3, "1B4AF05B-674A-4B68-8C72-1B46644DA0EC");
