#include "TextureAsset.h"

#include "Core/Asset.h"
#include "Core/Serialisation.h"
#include "System/Log.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <string_view>

namespace Columbus
{

	namespace
	{
		static std::string ToLowerCopy(std::string Value)
		{
			std::transform(Value.begin(), Value.end(), Value.begin(), [](unsigned char C) { return (char)tolower(C); });
			return Value;
		}

		static TextureFormat CompressionModeToTextureFormat(CompressionMode Mode)
		{
			switch (Mode)
			{
			case CompressionMode::None: return TextureFormat::Unknown;
			case CompressionMode::BC1: return TextureFormat::BC1;
			case CompressionMode::BC3: return TextureFormat::BC3;
			case CompressionMode::BC5: return TextureFormat::BC5;
			case CompressionMode::BC6H: return TextureFormat::BC6H;
			case CompressionMode::BC7: return TextureFormat::BC7;
			default: return TextureFormat::Unknown;
			}
		}

		static std::string NormalisePathSeparators(std::string Path)
		{
			std::replace(Path.begin(), Path.end(), '\\', '/');
			return Path;
		}

		static std::string MakeTextureSourcePathForAsset(const char* SourcePath)
		{
			if (SourcePath == nullptr)
				return {};

			std::string Result = NormalisePathSeparators(SourcePath);
			AssetSystem& Assets = AssetSystem::Get();

			if (!Assets.SourceDataPath.empty() && Assets.IsPathInSourceFolder(Result))
			{
				return Assets.MakePathRelativeToSourceFolder(Result);
			}

			return Result;
		}

		static u32 GetLayerCount(ImageType Type)
		{
			return Type == ImageType::ImageCube ? 6u : 1u;
		}

		static void SetImageView(Image& Img, u32 Width, u32 Height, u32 Depth, u32 Mips, TextureFormat Format, ImageType Type, const Blob& Pixels)
		{
			Img.Width = Width;
			Img.Height = Height;
			Img.Depth = Depth;
			Img.MipMaps = Mips;
			Img.Format = Format;
			Img.Type = Type;
			Img.Exist = !Pixels.Empty();
			Img.Size = Pixels.Size();
			Img.Data = const_cast<u8*>(Pixels.Data());
		}

		static void CopyImageToBlob(const Image& Img, Blob& OutPixels)
		{
			const u64 ByteCount = Img.GetFullSize();
			OutPixels.Bytes.resize((size_t)ByteCount);
			if (ByteCount > 0)
			{
				memcpy(OutPixels.Bytes.data(), Img.GetData(), (size_t)ByteCount);
			}
		}

		static void NormaliseUploadImage(Image& Img);

		static void ExtractMipRange(const Image& SourceImage, u32 FirstMip, u32 MipCount, Image& OutImage)
		{
			const u32 NewWidth = std::max(1u, SourceImage.Width >> FirstMip);
			const u32 NewHeight = std::max(1u, SourceImage.Height >> FirstMip);
			const u32 NewDepth = std::max(1u, SourceImage.Depth >> FirstMip);
			OutImage.AllocImage(NewWidth, NewHeight, NewDepth, MipCount, SourceImage.Format, SourceImage.Type);

			const u32 LayerCount = GetLayerCount(SourceImage.Type);
			for (u32 Layer = 0; Layer < LayerCount; Layer++)
			{
				for (u32 Mip = 0; Mip < MipCount; Mip++)
				{
					const u32 SourceMip = FirstMip + Mip;
					const u64 ByteCount = SourceImage.GetSize(SourceMip);
					memcpy(
						OutImage.Data + OutImage.GetOffset(Layer, Mip),
						SourceImage.Data + SourceImage.GetOffset(Layer, SourceMip),
						(size_t)ByteCount);
				}
			}
		}

		static Image BuildTextureCookBase(const Image& SourceImage, const TextureImportSettings& Settings)
		{
			Image WorkingImage;
			WorkingImage = SourceImage;
			NormaliseUploadImage(WorkingImage);
			WorkingImage.Format = TextureFormatRemoveSrgb(WorkingImage.Format);

			const bool NeedsMipChain = Settings.MipGen != MipGenMode::None || (Settings.MaxSize > 0 &&
				(std::max({ WorkingImage.Width, WorkingImage.Height, WorkingImage.Depth }) > (u32)Settings.MaxSize));

			Image ImageWithMips;
			Image* CurrentImage = &WorkingImage;
			if (NeedsMipChain)
			{
				ImageMips::GenerateImageMips(WorkingImage, ImageWithMips, {});
				CurrentImage = &ImageWithMips;
			}

			u32 FirstMip = 0;
			if (Settings.MaxSize > 0)
			{
				while (FirstMip + 1 < CurrentImage->MipMaps &&
					(std::max({ std::max(1u, CurrentImage->Width >> FirstMip), std::max(1u, CurrentImage->Height >> FirstMip), std::max(1u, CurrentImage->Depth >> FirstMip) }) > (u32)Settings.MaxSize))
				{
					FirstMip++;
				}
			}

			const u32 OutputMipCount = Settings.MipGen == MipGenMode::None ? 1u : (CurrentImage->MipMaps - FirstMip);

			Image CookBase;
			ExtractMipRange(*CurrentImage, FirstMip, OutputMipCount, CookBase);
			return CookBase;
		}

		static bool DecodeImageForSourceStorage(const Image& LoadedImage, Image& OutDecoded)
		{
			if (TextureFormatGetInfo(LoadedImage.GetFormat()).HasCompression)
			{
				if (!ImageCompression::DecompressImage(LoadedImage, OutDecoded))
				{
					Log::Error("Couldn't decompress imported image");
					return false;
				}
				return true;
			}

			OutDecoded = LoadedImage;
			return true;
		}

		static bool PathContainsAny(const std::string& Value, std::initializer_list<const char*> Needles)
		{
			for (const char* Needle : Needles)
			{
				if (Value.find(Needle) != std::string::npos)
					return true;
			}
			return false;
		}

		static bool StemEndsWithAny(const std::string& Stem, std::initializer_list<const char*> Suffixes)
		{
			for (const char* Suffix : Suffixes)
			{
				const size_t SuffixLength = strlen(Suffix);
				if (Stem.size() >= SuffixLength && Stem.compare(Stem.size() - SuffixLength, SuffixLength, Suffix) == 0)
					return true;
			}
			return false;
		}

		static void ApplyDetectedTextureImportSettings(Texture2& Texture, const Image& DecodedSource, const char* SourcePath)
		{
			const TextureFormatInfo FormatInfo = TextureFormatGetInfo(DecodedSource.Format);
			const std::filesystem::path PathObject = SourcePath ? std::filesystem::path(SourcePath) : std::filesystem::path();
			const std::string LowerPath = ToLowerCopy(NormalisePathSeparators(PathObject.generic_string()));
			const std::string LowerStem = ToLowerCopy(PathObject.stem().string());
			const std::string LowerExtension = ToLowerCopy(PathObject.extension().string());
			const bool bHasMeaningfulAlpha = ImageUtils::HasMeaningfulAlpha(DecodedSource);

			const bool bLikelyHdr = LowerExtension == ".hdr" || LowerExtension == ".exr" || TextureFormatIsHdr(DecodedSource.Format);
			const bool bLikelyNormal = PathContainsAny(LowerPath, { "normal", "normals", "nrm", "normalgl", "normaldx" }) ||
				StemEndsWithAny(LowerStem, { "_n", "-n", "_normal", "-normal", "_nrm", "-nrm" });
			const bool bLikelyUi = PathContainsAny(LowerPath, { "/ui/", "/hud/", "icon", "icons", "sprite", "sprites", "font", "fonts", "atlas", "atlases", "cursor" });
			const bool bLikelyDataMask = PathContainsAny(LowerPath, {
				"orm", "metalrough", "metallicroughness", "roughness", "rough", "metalness", "metallic",
				"occlusion", "_ao", "-ao", "ao_", "mask", "masks", "opacity", "specular", "gloss", "rma", "mra", "arm"
				}) || StemEndsWithAny(LowerStem, { "_orm", "-orm", "_rma", "-rma", "_mra", "-mra", "_arm", "-arm", "_ao", "-ao", "_rough", "-rough", "_metal", "-metal" });

			Texture.ImportSettings.AddressMode = TextureAddressMode::Repeat;
			Texture.ImportSettings.MaxSize = 0;

			if (bLikelyHdr)
			{
				Texture.ImportSettings.ColourSpace = ColourSpaceMode::Linear;
				Texture.ImportSettings.Compression = CompressionMode::BC6H;
				Texture.ImportSettings.MipGen = MipGenMode::Default;
			}
			else if (bLikelyNormal)
			{
				Texture.ImportSettings.ColourSpace = ColourSpaceMode::Linear;
				Texture.ImportSettings.Compression = CompressionMode::BC5;
				Texture.ImportSettings.MipGen = MipGenMode::NormalMap;
			}
			else if (bLikelyUi)
			{
				Texture.ImportSettings.ColourSpace = ColourSpaceMode::SRGB;
				Texture.ImportSettings.Compression = CompressionMode::None;
				Texture.ImportSettings.MipGen = MipGenMode::None;
				Texture.ImportSettings.AddressMode = TextureAddressMode::ClampToEdge;
			}
			else if (bLikelyDataMask || !TextureFormatIsLikelySrgbSource(DecodedSource.Format))
			{
				Texture.ImportSettings.ColourSpace = ColourSpaceMode::Linear;
				Texture.ImportSettings.Compression = bHasMeaningfulAlpha ? CompressionMode::BC7 : CompressionMode::BC1;
				Texture.ImportSettings.MipGen = MipGenMode::Default;
			}
			else
			{
				Texture.ImportSettings.ColourSpace = ColourSpaceMode::SRGB;
				Texture.ImportSettings.Compression = bHasMeaningfulAlpha ? CompressionMode::BC7 : CompressionMode::BC1;
				Texture.ImportSettings.MipGen = MipGenMode::Default;
			}
		}

		static void NormaliseUploadImage(Image& Img)
		{
			switch (Img.Format)
			{
			case TextureFormat::RGB8:
			case TextureFormat::RGB8SRGB:
			{
				Image Converted;
				Converted.AllocImage(Img.Width, Img.Height, Img.Depth, Img.MipMaps, TextureFormat::RGBA8, Img.Type);

				for (u32 Layer = 0; Layer < (Img.Type == ImageType::ImageCube ? 6u : 1u); Layer++)
				{
					for (u32 Mip = 0; Mip < Img.MipMaps; Mip++)
					{
						ImageMip Src = Img.GetMip(Mip, Layer);
						ImageMip Dst = Converted.GetMip(Mip, Layer);
						const u64 PixelCount = (u64)Src.Width * (u64)Src.Height * (u64)std::max(1u, Src.Depth);
						for (u64 Pixel = 0; Pixel < PixelCount; Pixel++)
						{
							const u8* SrcPixel = Src.Data + Pixel * 3;
							u8* DstPixel = Dst.Data + Pixel * 4;
							DstPixel[0] = SrcPixel[0];
							DstPixel[1] = SrcPixel[1];
							DstPixel[2] = SrcPixel[2];
							DstPixel[3] = 255;
						}
					}
				}

				Img = std::move(Converted);
				break;
			}
			default:
				break;
			}
		}

		static bool PopulateTextureSourceData(Texture2& Texture, const Image& SourceImage, const char* SourcePath)
		{
			Image DecodedSource;
			if (!DecodeImageForSourceStorage(SourceImage, DecodedSource))
				return false;

			Texture.SourcePath = MakeTextureSourcePathForAsset(SourcePath);
			Texture.Source.Width = DecodedSource.Width;
			Texture.Source.Height = DecodedSource.Height;
			Texture.Source.Depth = DecodedSource.Depth;
			Texture.Source.Mips = DecodedSource.MipMaps;
			Texture.Source.Format = TextureFormatGetInfo(DecodedSource.Format).FriendlyName;
			Texture.Source.Type = ImageTypeToString(DecodedSource.Type);
			CopyImageToBlob(DecodedSource, Texture.Source.Pixels);

			if (!Texture.Source.Pixels.Empty())
			{
				ApplyDetectedTextureImportSettings(Texture, DecodedSource, SourcePath);
			}

			return true;
		}

		static bool RecookTextureAssetData(Texture2& Texture)
		{
			if (Texture.Source.Pixels.Empty())
				return false;

			Image SourceImage;
			SetImageView(
				SourceImage,
				Texture.Source.Width,
				Texture.Source.Height,
				Texture.Source.Depth,
				Texture.Source.Mips,
				TextureFormatFromFriendlyName(Texture.Source.Format),
				ImageTypeFromString(Texture.Source.Type),
				Texture.Source.Pixels);

			Image CookedImage = BuildTextureCookBase(SourceImage, Texture.ImportSettings);

			if (Texture.ImportSettings.Compression != CompressionMode::None)
			{
				Image CompressedImage;
				ImageCompression::CompressionParams Params
				{
					.Format = CompressionModeToTextureFormat(Texture.ImportSettings.Compression),
					.PerceptualMetric = Texture.ImportSettings.ColourSpace == ColourSpaceMode::SRGB,
				};

				if (!ImageCompression::CompressImage(CookedImage, CompressedImage, Params))
				{
					return false;
				}

				CookedImage = std::move(CompressedImage);
			}

			CookedImage.Format = TextureFormatApplySrgb(CookedImage.Format, Texture.ImportSettings.ColourSpace == ColourSpaceMode::SRGB);

			Texture.Cooked.Width = CookedImage.Width;
			Texture.Cooked.Height = CookedImage.Height;
			Texture.Cooked.Depth = CookedImage.Depth;
			Texture.Cooked.Mips = CookedImage.MipMaps;
			Texture.Cooked.Format = TextureFormatGetInfo(CookedImage.Format).FriendlyName;
			Texture.Cooked.Type = ImageTypeToString(CookedImage.Type);
			CopyImageToBlob(CookedImage, Texture.Cooked.Pixels);

			SourceImage.Data = nullptr;
			return true;
		}

		static Texture2* CreateRuntimeTextureFromCookedData(SPtr<DeviceVulkan> Device, const Texture2& TextureData)
		{
			if (TextureData.Cooked.Pixels.Empty())
				return nullptr;

			Image CookedImage;
			SetImageView(
				CookedImage,
				TextureData.Cooked.Width,
				TextureData.Cooked.Height,
				TextureData.Cooked.Depth,
				TextureData.Cooked.Mips,
				TextureFormatFromFriendlyName(TextureData.Cooked.Format),
				ImageTypeFromString(TextureData.Cooked.Type),
				TextureData.Cooked.Pixels);

			TextureDesc2 RuntimeDesc;
			RuntimeDesc.AddressU = TextureData.ImportSettings.AddressMode;
			RuntimeDesc.AddressV = TextureData.ImportSettings.AddressMode;
			RuntimeDesc.AddressW = TextureData.ImportSettings.AddressMode;

			Texture2* RuntimeTexture = Device->CreateTexture(CookedImage, RuntimeDesc);
			RuntimeTexture->CopyAssetFieldsFrom(TextureData);
			CookedImage.Data = nullptr;
			return RuntimeTexture;
		}

		static Texture2* LoadTextureAssetBinaryInternal(SPtr<DeviceVulkan> Device, const char* Path)
		{
			DataStream Stream = DataStream::CreateFromFile(Path, "rb");
			if (!Stream.IsValid())
			{
				Log::Error("Couldn't open texture asset %s", Path);
				return nullptr;
			}

			Texture2 TextureData;
			if (!Reflection_DeserialiseStructBinary(Stream, TextureData))
			{
				Log::Error("Couldn't deserialize texture asset %s", Path);
				return nullptr;
			}

			Texture2* RuntimeTexture = CreateRuntimeTextureFromCookedData(Device, TextureData);
			if (RuntimeTexture)
			{
				Device->SetDebugName(RuntimeTexture, std::filesystem::path(Path).filename().string().c_str());
			}
			return RuntimeTexture;
		}

		static Texture2* LoadLegacyTextureAssetInternal(SPtr<DeviceVulkan> Device, const char* Path)
		{
			Image SourceImage;
			if (!SourceImage.LoadFromFile(Path))
				return nullptr;

			Texture2 TextureData;
			if (!PopulateTextureSourceData(TextureData, SourceImage, Path))
				return nullptr;
			TextureData.ImportSettings.ColourSpace = ColourSpaceMode::Linear;
			TextureData.ImportSettings.Compression = CompressionMode::None;
			TextureData.ImportSettings.MipGen = MipGenMode::None;
			TextureData.ImportSettings.MaxSize = 0;

			if (!RecookTextureAssetData(TextureData))
				return nullptr;

			Texture2* RuntimeTexture = CreateRuntimeTextureFromCookedData(Device, TextureData);
			if (RuntimeTexture)
			{
				Device->SetDebugName(RuntimeTexture, std::filesystem::path(Path).filename().string().c_str());
			}
			return RuntimeTexture;
		}
	}

	Texture2* LoadTextureAssetForRuntime(SPtr<DeviceVulkan> Device, const char* Path)
	{
		const std::string Extension = ToLowerCopy(std::filesystem::path(Path).extension().string());
		if (Extension == ".cas")
		{
			return LoadTextureAssetBinaryInternal(Device, Path);
		}

		return LoadLegacyTextureAssetInternal(Device, Path);
	}

	Texture2* ImportTextureAssetFromSource(SPtr<DeviceVulkan> Device, const char* SourcePath)
	{
		Image SourceImage;
		if (!SourceImage.LoadFromFile(SourcePath))
			return nullptr;

		Texture2 TextureData;
		if (!PopulateTextureSourceData(TextureData, SourceImage, SourcePath))
			return nullptr;

		if (!RecookTextureAssetData(TextureData))
			return nullptr;

		Texture2* RuntimeTexture = CreateRuntimeTextureFromCookedData(Device, TextureData);
		if (RuntimeTexture)
		{
			Device->SetDebugName(RuntimeTexture, std::filesystem::path(SourcePath).filename().string().c_str());
		}
		return RuntimeTexture;
	}

	Texture2* RebuildTextureAssetForEditor(SPtr<DeviceVulkan> Device, const Texture2& SourceAsset)
	{
		Texture2 TextureData;
		TextureData.CopyAssetFieldsFrom(SourceAsset);

		if (!RecookTextureAssetData(TextureData))
			return nullptr;

		Texture2* RuntimeTexture = CreateRuntimeTextureFromCookedData(Device, TextureData);
		if (RuntimeTexture)
		{
			const std::string Name = TextureData.SourcePath.empty() ? "TextureAsset" : std::filesystem::path(TextureData.SourcePath).filename().string();
			Device->SetDebugName(RuntimeTexture, Name.c_str());
		}
		return RuntimeTexture;
	}

	bool SaveTextureAssetToFile(const Texture2& Texture, const char* Path)
	{
		DataStream Stream = DataStream::CreateFromFile(Path, "wb");
		if (!Stream.IsValid())
			return false;

		return Reflection_SerialiseStructBinary(Stream, const_cast<Texture2&>(Texture));
	}

}
