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

		bool Load(const void* Data, Texture::Properties Props) override;
		bool Load(Image& InImage) override;
		bool Load(std::string File) override;

		void Clear() override;

		bool Create2D(Texture::Properties Props) override;
		bool CreateCube(Texture::Properties Props) override;

		void SetFlags(Texture::Flags F) override;

		void SetMipmapLevel(uint32 Level) override;

		void Bind();
		void Unbind();
		void Bind(uint32 Unit);
		void Unbind(uint32 Unit);

		void bind() override;
		void unbind() override;

		void sampler2D(int a) override;
		void generateMipmap() override;

		uint32 GetID() const; //Get OpenGL-specified texture ID
		uint32 GetTarget() const; //Get OpenGL-specified texture target
		uint32 GetInternalFormat() const; //Get OpenGL-specified texture internal format
		uint32 GetPixelFormat() const; //Get OpenGL-specified texture pixel format
		uint32 GetPixelType() const; //Get OpenGL-specified pixel type

		~TextureOpenGL() override;
	};

}






