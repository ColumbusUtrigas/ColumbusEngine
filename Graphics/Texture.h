#pragma once

#include <Common/Image/Image.h>
#include <memory>

namespace Columbus
{

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

		virtual bool Create(Image::Type InType, TextureDesc Desc) = 0;
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

	struct DefaultTextures
	{
		std::unique_ptr<Texture> Black;
		std::unique_ptr<Texture> White;
		std::unique_ptr<Texture> IntegrationLUT;

		// TODO: Load this textures only if EditorMode is enabled.
		std::unique_ptr<Texture> IconSun;
		std::unique_ptr<Texture> IconLamp;
		std::unique_ptr<Texture> IconFlashlight;
		std::unique_ptr<Texture> IconAudio;
		std::unique_ptr<Texture> IconParticles;

		DefaultTextures();
	};

}


