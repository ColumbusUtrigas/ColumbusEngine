#pragma once

#include <Graphics/Texture.h>

namespace Columbus
{

	class TextureOpenGL : public Texture
	{
	private:
		uint32 ID;
		uint32 Target;
		uint32 InternalFormat;
		uint32 PixelFormat;
		uint32 PixelType;

		bool UpdateFormat(TextureFormat Format, bool& OutCompressed);
		bool UpdateParameters();
	public:
		TextureOpenGL();
		TextureOpenGL(const TextureOpenGL&) = delete;
		TextureOpenGL(TextureOpenGL&&) = delete;

		bool Load(const void* Data, Texture::Properties Props) override;
		bool Load(Image& InImage) override;
		bool Load(const char* File) override;

		bool Create(Image::Type InType, Properties Props) override;
		bool Create(Texture::Type InType, Properties Props) override;
		bool Create2D(Texture::Properties Props) override;
		bool CreateCube(Texture::Properties Props) override;

		void SetFlags(Texture::Flags F) override;

		void SetMipmapLevel(uint32 Level) override;

		void Bind();
		void Unbind();
		void Bind(uint32 Unit);
		void Unbind(uint32 Unit);

		void GenerateMipmap() override;

		//OpenGL specific
		uint32 GetID() const;
		uint32 GetTarget() const;
		uint32 GetInternalFormat() const;
		uint32 GetPixelFormat() const;
		uint32 GetPixelType() const;

		~TextureOpenGL() override;
	};

}


