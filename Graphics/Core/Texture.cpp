#include "Texture.h"

#include <algorithm>
#include <cstring>

namespace Columbus
{

	TextureDesc2 TextureView::GetTextureDesc() const
	{
		return TextureUtils::TextureViewTextureDesc(Texture->GetDesc(), Desc);
	}

	namespace TextureUtils
	{
		TextureType TextureTypeFromImageType(ImageType Type)
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

		ImageType ImageTypeFromTextureType(TextureType Type)
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

		u32 TextureLayerCount(TextureType Type, u32 ArrayLayers)
		{
			if (Type == TextureType::TextureCube)
			{
				return 6u;
			}

			if (Type == TextureType::TextureCubeArray ||
				Type == TextureType::Texture1DArray ||
				Type == TextureType::Texture2DArray)
			{
				return std::max(1u, ArrayLayers);
			}

			return 1u;
		}

		u32 TextureLayerCount(const TextureDesc2& Desc)
		{
			return TextureLayerCount(Desc.Type, Desc.ArrayLayers);
		}

		TextureDesc2 NormalizeTextureDesc(TextureDesc2 Desc)
		{
			Desc.Width       = std::max(1u, Desc.Width);
			Desc.Height      = std::max(1u, Desc.Height);
			Desc.Depth       = Desc.Type == TextureType::Texture3D ? std::max(1u, Desc.Depth) : 1u;
			Desc.Mips        = std::max(1u, Desc.Mips);
			Desc.ArrayLayers = TextureLayerCount(Desc);
			Desc.Samples     = std::max(1u, Desc.Samples);
			return Desc;
		}

		u32 TextureArrayLayersForImage(const Image& ImageData, u32 FallbackArrayLayers)
		{
			return ImageData.GetType() == ImageType::ImageCube ? ImageUtils::ImageLayerCount(ImageData.GetType()) : std::max(1u, FallbackArrayLayers);
		}

		void SetTextureDescImageFields(TextureDesc2& Desc, const Image& ImageData, u32 FallbackArrayLayers)
		{
			Desc.Type        = TextureTypeFromImageType(ImageData.GetType());
			Desc.Width       = std::max(1u, ImageData.GetWidth());
			Desc.Height      = std::max(1u, ImageData.GetHeight());
			Desc.Depth       = ImageData.GetType() == ImageType::Image3D ? std::max(1u, ImageData.GetDepth()) : 1u;
			Desc.Mips        = std::max(1u, ImageData.GetMipmapsCount());
			Desc.ArrayLayers = TextureArrayLayersForImage(ImageData, FallbackArrayLayers);
			Desc.Samples     = 1;
			Desc.Format      = ImageData.GetFormat();
		}

		void SetBorrowedImageViewFromStoredPixels(Image& ImageView, const TextureStoredPixels& Stored)
		{
			ImageView.Width   = (u32)Stored.Width;
			ImageView.Height  = (u32)Stored.Height;
			ImageView.Depth   = (u32)Stored.Depth;
			ImageView.MipMaps = (u32)Stored.Mips;
			ImageView.Format  = TextureFormatFromFriendlyName(Stored.Format);
			ImageView.Type    = ImageTypeFromString(Stored.Type);
			ImageView.Exist   = !Stored.Pixels.Empty();
			ImageView.Size    = Stored.Pixels.Size();
			ImageView.Data    = const_cast<u8*>(Stored.Pixels.Data());
		}

		void StoreImagePixels(TextureStoredPixels& Stored, const Image& ImageData)
		{
			Stored.Width  = (int)ImageData.Width;
			Stored.Height = (int)ImageData.Height;
			Stored.Depth  = (int)std::max(1u, ImageData.Depth);
			Stored.Mips   = (int)std::max(1u, ImageData.MipMaps);
			Stored.Format = TextureFormatGetInfo(ImageData.Format).FriendlyName;
			Stored.Type   = ImageTypeToString(ImageData.Type);
			Stored.Pixels.Bytes.resize((size_t)ImageData.GetFullSize());
			if (!Stored.Pixels.Bytes.empty())
			{
				memcpy(Stored.Pixels.Data(), ImageData.Data, Stored.Pixels.Size());
			}
		}

		u64 ComputeStoredPixelByteSize(const TextureStoredPixels& Stored)
		{
			if (Stored.Width <= 0 || Stored.Height <= 0 || Stored.Depth <= 0 || Stored.Mips <= 0)
			{
				return 0;
			}

			const TextureFormat Format = TextureFormatFromFriendlyName(Stored.Format);
			if (Format == TextureFormat::Unknown)
			{
				return 0;
			}

			const ImageType Type = ImageTypeFromString(Stored.Type);
			return ImageUtils::ImageCalcByteSize((u32)Stored.Width, (u32)Stored.Height, (u32)Stored.Depth, (u32)Stored.Mips, Format, Type);
		}

		bool ValidateStoredPixelByteSize(const TextureStoredPixels& Stored, u64* OutExpectedBytes)
		{
			const u64 ExpectedBytes = ComputeStoredPixelByteSize(Stored);
			if (OutExpectedBytes != nullptr)
			{
				*OutExpectedBytes = ExpectedBytes;
			}
			return Stored.Pixels.Size() == ExpectedBytes;
		}

		// Resolve remaining counts once, before using a view for bindings or dependency tracking.
		bool ResolveTextureViewDesc(const TextureDesc2& Texture, TextureViewDesc& View)
		{
			const u32 Mips   = std::max(1u, Texture.Mips);
			const u32 Layers = TextureLayerCount(Texture);

			if (View.BaseMip >= Mips || View.BaseLayer >= Layers)
			{
				return false;
			}

			View.MipCount   = View.MipCount == 0 ? Mips - View.BaseMip : View.MipCount;
			View.LayerCount = View.LayerCount == 0 ? Layers - View.BaseLayer : View.LayerCount;

			if (View.MipCount > Mips - View.BaseMip || View.LayerCount > Layers - View.BaseLayer)
			{
				return false;
			}

			const bool Source1D = Texture.Type == TextureType::Texture1D || Texture.Type == TextureType::Texture1DArray;
			const bool View1D   = View.ViewType == TextureType::Texture1D || View.ViewType == TextureType::Texture1DArray;
			const bool Source3D = Texture.Type == TextureType::Texture3D;
			const bool View3D   = View.ViewType == TextureType::Texture3D;
			const bool CubeView = View.ViewType == TextureType::TextureCube || View.ViewType == TextureType::TextureCubeArray;
			const bool CubeSource = Texture.Type == TextureType::TextureCube || Texture.Type == TextureType::TextureCubeArray;
			const bool SingleLayer = View.ViewType == TextureType::Texture1D || View.ViewType == TextureType::Texture2D || View3D;

			if (Source1D != View1D || Source3D != View3D || (SingleLayer && View.LayerCount != 1) ||
				(CubeView && (!CubeSource || View.BaseLayer % 6 != 0 || View.LayerCount % 6 != 0)) ||
				(View.ViewType == TextureType::TextureCube && View.LayerCount != 6))
			{
				return false;
			}

			const auto Format = TextureFormatGetInfo(Texture.Format);
			const u32 Aspect  = u32(View.Aspect);
			return Aspect == u32(TextureBindingFlags::AspectColour) || (Aspect != 0 && Aspect <= 3 && (!(Aspect & 1) || Format.HasDepth) && (!(Aspect & 2) || Format.HasStencil));
		}

		TextureDesc2 TextureViewTextureDesc(TextureDesc2 Texture, const TextureViewDesc& View)
		{
			Texture.Width       = std::max(1u, Texture.Width >> View.BaseMip);
			Texture.Height      = std::max(1u, Texture.Height >> View.BaseMip);
			Texture.Depth       = Texture.Type == TextureType::Texture3D ? std::max(1u, Texture.Depth >> View.BaseMip) : 1u;
			Texture.Type        = View.ViewType;
			Texture.Mips        = View.MipCount;
			Texture.ArrayLayers = View.LayerCount;
			return Texture;
		}

	}

}
