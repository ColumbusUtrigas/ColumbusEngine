#pragma once

#include <Common/Image/Image.h>

namespace Columbus
{

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
		virtual bool Load(const char* File) = 0;

		virtual void Clear() = 0;

		virtual bool Create2D(Properties Props) = 0;
		virtual bool CreateCube(Properties Props) = 0;

		virtual void SetFlags(Flags F) = 0;

		virtual void SetMipmapLevel(uint32 Level) = 0;
		uint32 GetMipmapLevel() const { return MipmapLevel; }
		
		virtual void bind() = 0;
		virtual void unbind() = 0;

		virtual void sampler2D(int a) = 0;
		virtual void generateMipmap() = 0;
		
		virtual ~Texture() {}
	protected:
		Flags TextureFlags;
	};

}




