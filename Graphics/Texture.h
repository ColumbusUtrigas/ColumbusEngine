#pragma once

#include <Common/Image/Image.h>

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

		virtual void SetFlags(Flags F) = 0;

		uint32 GetMultisampling() const { return Multisampling; }
		TextureFormat GetFormat() const { return Format; }
		Type GetType() const { return TextureType; }

		virtual void SetMipmapLevel(uint32 Level) = 0;
		uint32 GetMipmapLevel() const { return MipmapLevel; }

		virtual void GenerateMipmap() = 0;
		
		virtual ~Texture() {}
	protected:
		Flags TextureFlags;
	};

	struct DefaultTextures
	{
		Texture* Black = nullptr;
		Texture* White = nullptr;
		Texture* IntegrationLUT = nullptr;


		// TODO: Load this textures only if EditorMode is enabled.
		Texture* IconSun = nullptr;
		Texture* IconLamp = nullptr;
		Texture* IconFlashlight = nullptr;
		Texture* IconAudio = nullptr;
		Texture* IconParticles = nullptr;

		DefaultTextures();
		~DefaultTextures();
	};

}


