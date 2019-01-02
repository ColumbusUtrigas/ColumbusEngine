#pragma once

#include <Common/Image/Image.h>
#include <Math/Vector2.h>

namespace Columbus
{

	struct TextureConfig
	{
		bool smooth = true;
		bool mipmaps = true;
		unsigned int anisotropy = 8;
		unsigned int LOD = 0;
		Vector2 tiling = Vector2(1, 1);
		Vector2 tilingOffset = Vector2(0, 0);
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
		Image mImage;

		std::string mFile;

		TextureConfig mConfig;

		uint32 Width;
		uint32 Height;

		uint32 MipmapsCount;
		uint32 MipmapLevel;

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
			ClampToEdge,
			Repeat,
			MirroredRepeat,
			MirroredClampToEdge
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
		};

		struct Properties
		{
			uint32 Width = 0;
			uint32 Height = 0;
			uint32 LOD = 0;
			TextureFormat Format;

			Properties(uint32 InWidth, uint32 InHeight, uint32 InLOD, TextureFormat InFormat) :
				Width(InWidth),
				Height(InHeight),
				LOD(InLOD),
				Format(InFormat) {}
		};
	public:
		Texture() {}
		Texture(const Texture&) = delete;
		Texture(Texture&&) = delete;

		virtual bool Load(const void* Data, Properties Props) = 0;
		virtual bool Load(Image& InImage) = 0;
		virtual bool Load(std::string File) = 0;

		virtual void Clear() = 0;

		virtual bool Create2D(Properties Props) = 0;
		virtual bool CreateCube(Properties Props) = 0;

		virtual void SetFlags(Flags F) = 0;

		virtual void SetMipmapLevel(uint32 Level) = 0;
		uint32 GetMipmapLevel() const { return MipmapLevel; }

		TextureConfig GetConfig() const;

		bool save(std::string aFile, size_t aQuality = 100);
		
		virtual void bind();
		virtual void unbind();

		virtual void sampler2D(int a);
		virtual void generateMipmap();
		
		virtual ~Texture();
	protected:
		Flags TextureFlags;
	};

}




