#include <Graphics/OpenGL/TextureOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	TextureOpenGL::TextureOpenGL()
	{
		//glGenTextures(1, &ID);
		//Target = GL_TEXTURE_2D;
	}

	bool TextureOpenGL::UpdateFormat(TextureFormat Format, bool& OutCompressed)
	{
		OutCompressed = false;

		switch (Format)
		{
			//8 Bit
			case TextureFormat::R8:
			{
				InternalFormat = GL_R8;
				PixelFormat = GL_RED;
				PixelType = GL_UNSIGNED_BYTE;
				return true;
				break;
			}

			case TextureFormat::RG8:
			{
				InternalFormat = GL_RG8;
				PixelFormat = GL_RG;
				PixelType = GL_UNSIGNED_BYTE;
				return true;
				break;
			}

			case TextureFormat::RGB8:
			{
				InternalFormat = GL_RGB8;
				PixelFormat = GL_RGB;
				PixelType = GL_UNSIGNED_BYTE;
				return true;
				break;
			}

			case TextureFormat::RGBA8:
			{
				InternalFormat = GL_RGBA8;
				PixelFormat = GL_RGBA;
				PixelType = GL_UNSIGNED_BYTE;
				return true;
				break;
			}
			//16 Bit
			case TextureFormat::R16:
			{
				InternalFormat = GL_R16;
				PixelFormat = GL_RED;
				PixelType = GL_UNSIGNED_SHORT;
				return true;
				break;
			}

			case TextureFormat::RG16:
			{
				InternalFormat = GL_RG16;
				PixelFormat = GL_RG;
				PixelType = GL_UNSIGNED_SHORT;
				return true;
				break;
			}

			case TextureFormat::RGB16:
			{
				InternalFormat = GL_RGB16;
				PixelFormat = GL_RGB;
				PixelType = GL_UNSIGNED_SHORT;
				return true;
				break;
			}

			case TextureFormat::RGBA16:
			{
				InternalFormat = GL_RGBA16;
				PixelFormat = GL_RGBA;
				PixelType = GL_UNSIGNED_SHORT;
				return true;
				break;
			}
			//16 Bit Float
			case TextureFormat::R16F:
			{
				InternalFormat = GL_R16F;
				PixelFormat = GL_RED;
				PixelType = GL_UNSIGNED_SHORT;
				return true;
				break;
			}

			case TextureFormat::RG16F:
			{
				InternalFormat = GL_RG16F;
				PixelFormat = GL_RG;
				PixelType = GL_UNSIGNED_SHORT;
				return true;
				break;
			}

			case TextureFormat::RGB16F:
			{
				InternalFormat = GL_RGB16F;
				PixelFormat = GL_RGB;
				PixelType = GL_UNSIGNED_SHORT;
				return true;
				break;
			}

			case TextureFormat::RGBA16F:
			{
				InternalFormat = GL_RGBA16F;
				PixelFormat = GL_RGBA;
				PixelType = GL_UNSIGNED_SHORT;
				return true;
				break;
			}
			//32 Bit Float
			case TextureFormat::R32F:
			{
				InternalFormat = GL_R32F;
				PixelFormat = GL_RED;
				PixelType = GL_FLOAT;
				return true;
				break;
			}

			case TextureFormat::RG32F:
			{
				InternalFormat = GL_RG32F;
				PixelFormat = GL_RG;
				PixelType = GL_FLOAT;
				return true;
				break;
			}

			case TextureFormat::RGB32F:
			{
				InternalFormat = GL_RGB32F;
				PixelFormat = GL_RGB;
				PixelType = GL_FLOAT;
				return true;
				break;
			}

			case TextureFormat::RGBA32F:
			{
				InternalFormat = GL_RGBA32F;
				PixelFormat = GL_RGBA;
				PixelType = GL_FLOAT;
				return true;
				break;
			}
			//Compressed
			case TextureFormat::DXT1:
			{
				InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				PixelType = GL_UNSIGNED_BYTE;
				OutCompressed = true;
				return true;
				break;
			}

			case TextureFormat::DXT3:
			{
				InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				PixelType = GL_UNSIGNED_BYTE;
				OutCompressed = true;
				return true;
				break;
			}

			case TextureFormat::DXT5:
			{
				InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				PixelType = GL_UNSIGNED_BYTE;
				OutCompressed = true;
				return true;
				break;
			}

			case TextureFormat::Depth:
			{
				InternalFormat = GL_DEPTH_COMPONENT;
				PixelFormat = GL_DEPTH_COMPONENT;
				PixelType = GL_FLOAT;
				return true;
				break;
			}

			case TextureFormat::Depth16:
			{
				InternalFormat = GL_DEPTH_COMPONENT16;
				PixelFormat = GL_DEPTH_COMPONENT;
				PixelType = GL_UNSIGNED_SHORT;
				return true;
				break;
			}

			case TextureFormat::Depth24:
			{
				InternalFormat = GL_DEPTH_COMPONENT24;
				PixelFormat = GL_DEPTH_COMPONENT;
				PixelType = GL_UNSIGNED_INT;
				return true;
				break;
			}

			case TextureFormat::Depth24Stencil8:
			{
				InternalFormat = GL_DEPTH24_STENCIL8;
				PixelFormat = GL_DEPTH_STENCIL;
				PixelType = GL_UNSIGNED_INT_24_8;
				return true;
				break;
			}

			case TextureFormat::Depth32F:
			{
				InternalFormat = GL_DEPTH_COMPONENT32F;
				PixelFormat = GL_DEPTH_COMPONENT;
				PixelType = GL_FLOAT;
				return true;
				break;
			}

			case TextureFormat::Depth32FStencil8:
			{
				InternalFormat = GL_DEPTH32F_STENCIL8;
				PixelFormat = GL_DEPTH_STENCIL;
				PixelType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
				return true;
				break;
			}

			case TextureFormat::Unknown:
			{
				return false;
			}
		}

		return false;
	}

	bool TextureOpenGL::UpdateParameters()
	{


		return true;
	}

	bool TextureOpenGL::Load(const void* Data)
	{	
		glBindTexture(Target, ID);

		bool Compressed;

		UpdateFormat(Format, Compressed);

		if (Compressed)
		{
			glCompressedTexImage2D(Target, 0, InternalFormat, Width, Height, 0, Size, Data);
		}
		else
		{
			glTexImage2D(Target, 0, InternalFormat, Width, Height, 0, PixelFormat, PixelType, Data);
		}

		glGenerateMipmap(Target);

		Flags f;
		f.Filtering = Texture::Filter::Trilinear;
		f.AnisotropyFilter = Texture::Anisotropy::Anisotropy8;

		SetFlags(f);

		glBindTexture(Target, 0);

		return true;
	}

	bool TextureOpenGL::Load(const void* Data, Texture::Properties Props)
	{
		if (glIsTexture(ID))
		{
			Width = Props.Width;
			Height = Props.Height;
			Size = Props.Size;
			Format = Props.Format;

			bool Compressed;

			UpdateFormat(Format, Compressed);

			glBindTexture(Target, ID);
			glTexImage2D(Target, 0, InternalFormat, Width, Height, 0, PixelFormat, PixelType, Data);
			glBindTexture(Target, 0);

			return true;
		}

		return false;
	}

	bool TextureOpenGL::Load(Image& InImage)
	{
		if (glIsTexture(ID))
		{
			bool Success = true;
			bool Compressed;

			UpdateFormat(Format, Compressed);

			glBindTexture(Target, ID);

			if (Compressed)
			{
				glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, InImage.GetMipmapsCount() - 1);
				glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

				for (uint32 Level = 0; Level < InImage.GetMipmapsCount(); Level++)
				{
					glCompressedTexImage2D(Target, Level, InternalFormat, Width, Height, 0, InImage.GetSize(Level), InImage.Get2DData(Level));
				}
			}
			else
			{
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexImage2D(Target, 0, InternalFormat, Width, Height, 0, PixelFormat, PixelType, InImage.Get2DData(0));
				glGenerateMipmap(Target);
			}

			Flags f;
			f.Filtering = Texture::Filter::Trilinear;
			f.AnisotropyFilter = Texture::Anisotropy::Anisotropy8;

			SetFlags(f);

			glBindTexture(Target, 0);

			return Success;
		}

		return false;
	}

	bool TextureOpenGL::Load(std::string File)
	{
		if (!mImage.Load(File))
		{
			return false;
		}

		bool Result = Load(mImage.GetData());

		mImage.FreeData();
		return Result;
	}

	void TextureOpenGL::Clear()
	{
		if (glIsTexture(ID))
		{
			glDeleteTextures(1, &ID);
		}

		Width = 0;
		Height = 0;
		Size = 0;
		MipmapsCount = 0;
		MipmapLevel = 0;

		Format = TextureFormat::Unknown;
		TextureType = Type::Texture2D;

		ID = 0;
		Target = 0;
		InternalFormat = 0;
		PixelFormat = 0;
		PixelType = 0;
	}
	
	bool TextureOpenGL::Create2D(Texture::Properties Props)
	{
		Clear();

		glGenTextures(1, &ID);

		TextureType = Texture::Type::Texture2D;

		Target = GL_TEXTURE_2D;
		Width = Props.Width;
		Height = Props.Height;
		Size = Props.Size;
		Format = Props.Format;

		bool Compressed;

		UpdateFormat(Format, Compressed);

		glBindTexture(Target, ID);

		glTexImage2D(Target, 0, InternalFormat, Width, Height, 0, PixelFormat, PixelType, 0);
		glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glBindTexture(Target, 0);

		return true;
	}

	bool TextureOpenGL::CreateCube(Texture::Properties Props)
	{
		Clear();

		glGenTextures(1, &ID);

		TextureType = Texture::Type::TextureCube;

		Target = GL_TEXTURE_CUBE_MAP;


		return false;
	}

	void TextureOpenGL::SetFlags(Texture::Flags F)
	{
		TextureFlags = F;

		glBindTexture(Target, ID);

		switch (TextureFlags.Filtering)
		{
			case Texture::Filter::Point:
			{
				glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				break;
			}

			case Texture::Filter::Linear:
			{
				glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;
			}

			case Texture::Filter::Bilinear:
			{
				glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;
			}

			case Texture::Filter::Trilinear:
			{
				glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;
			}
		}

		switch (TextureFlags.AnisotropyFilter)
		{
		case Texture::Anisotropy::Anisotropy1:  glTexParameteri(Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);  break;
		case Texture::Anisotropy::Anisotropy2:  glTexParameteri(Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2);  break;
		case Texture::Anisotropy::Anisotropy4:  glTexParameteri(Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4);  break;
		case Texture::Anisotropy::Anisotropy8:  glTexParameteri(Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);  break;
		case Texture::Anisotropy::Anisotropy16: glTexParameteri(Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16); break;
		default:                                glTexParameteri(Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);  break;
		}

		glBindTexture(Target, 0);
	}

	void TextureOpenGL::SetMipmapLevel(uint32 Level)
	{
		MipmapLevel = Level;

		glBindTexture(Target, ID);
		glTexParameteri(Target, GL_TEXTURE_MIN_LOD, Level);
	}


	void TextureOpenGL::Bind()
	{
		glBindTexture(Target, ID);
	}

	void TextureOpenGL::Unbind()
	{
		glBindTexture(Target, 0);
	}

	void TextureOpenGL::Bind(uint32 Unit)
	{
		glActiveTexture(GL_TEXTURE0 + Unit);
		glBindTexture(Target, ID);
	}

	void TextureOpenGL::Unbind(uint32 Unit)
	{
		glActiveTexture(GL_TEXTURE0 + Unit);
		glBindTexture(Target, 0);
	}

	void TextureOpenGL::bind()
	{
		glBindTexture(Target, ID);
	}
	
	void TextureOpenGL::unbind()
	{
		glBindTexture(Target, 0);
	}

	void TextureOpenGL::sampler2D(int a)
	{
		glActiveTexture(GL_TEXTURE0 + a);
		bind();
	}
	
	void TextureOpenGL::generateMipmap()
	{
		bind();
		glGenerateMipmap(Target);
		unbind();
	}

	uint32 TextureOpenGL::GetID() const
	{
		return ID;
	}

	uint32 TextureOpenGL::GetTarget() const
	{
		return Target;
	}

	uint32 TextureOpenGL::GetInternalFormat() const
	{
		return InternalFormat;
	}

	uint32 TextureOpenGL::GetPixelFormat() const
	{
		return PixelFormat;
	}

	uint32 TextureOpenGL::GetPixelType() const
	{
		return PixelType;
	}

	TextureOpenGL::~TextureOpenGL()
	{
		glDeleteTextures(1, &ID);
	}

}









