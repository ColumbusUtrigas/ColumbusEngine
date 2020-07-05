#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Math/MathUtil.h>
#include <GL/glew.h>

namespace Columbus
{

	TextureOpenGL::TextureOpenGL()
	{
		glGenTextures(1, &ID);
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
				PixelType = GL_FLOAT;
				return true;
				break;
			}

			case TextureFormat::RG16F:
			{
				InternalFormat = GL_RG16F;
				PixelFormat = GL_RG;
				PixelType = GL_FLOAT;
				return true;
				break;
			}

			case TextureFormat::RGB16F:
			{
				InternalFormat = GL_RGB16F;
				PixelFormat = GL_RGB;
				PixelType = GL_FLOAT;
				return true;
				break;
			}

			case TextureFormat::RGBA16F:
			{
				InternalFormat = GL_RGBA16F;
				PixelFormat = GL_RGBA;
				PixelType = GL_FLOAT;
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
			//Special
			case TextureFormat::R11G11B10F:
			{
				InternalFormat = GL_R11F_G11F_B10F;
				PixelFormat = GL_RGB;
				PixelType = GL_FLOAT;
				return true;
				break;
			}
			//Compressed
			case TextureFormat::DXT1:
			{
				InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				PixelFormat = GL_RGBA;
				PixelType = GL_UNSIGNED_BYTE;
				OutCompressed = true;
				return true;
				break;
			}

			case TextureFormat::DXT3:
			{
				InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				PixelFormat = GL_RGBA;
				PixelType = GL_UNSIGNED_BYTE;
				OutCompressed = true;
				return true;
				break;
			}

			case TextureFormat::DXT5:
			{
				InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				PixelFormat = GL_RGBA;
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


	bool TextureOpenGL::Load(const void* Data, TextureDesc Desc)
	{
		if (Target == GL_TEXTURE_2D)
		{
			Width = Desc.Width;
			Height = Desc.Height;
			Format = Desc.Format;
			MipmapsCount = 1;

			bool Compressed;

			UpdateFormat(Format, Compressed);
			Multisampling = 0;

			glBindTexture(Target, ID);

			if (Compressed)
			{
				auto Block = GetBlockSizeFromFormat(Format);
				glCompressedTexImage2D(Target, 0, InternalFormat, Width, Height, 0, Width * Height * Block / 16, Data);
			}
			else
			{
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexImage2D(Target, 0, InternalFormat, Width, Height, 0, PixelFormat, PixelType, Data);
			}
			
			glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				
			glBindTexture(Target, 0);

		}

		return true;
	}

	bool TextureOpenGL::Load(Image& InImage)
	{
		bool Success = true;
		bool Compressed = false;

		UpdateFormat(Format, Compressed);
		Multisampling = 0;

		glBindTexture(Target, ID);

		if (Compressed)
		{
			switch (InImage.GetType())
			{
				case Image::Type::Image2D:
				{
					for (uint32 Level = 0; Level < InImage.GetMipmapsCount(); Level++)
					{
						glCompressedTexImage2D(Target, Level, InternalFormat, Width >> Level, Height >> Level, 0, InImage.GetSize(Level), InImage.Get2DData(Level));
					}
					MipmapsCount = InImage.GetMipmapsCount();

					break;
				}

				case Image::Type::Image3D: break;

				case Image::Type::ImageCube:
				{
					for (uint32 Face = 0; Face < 6; Face++)
					{
						for (uint32 Level = 0; Level < InImage.GetMipmapsCount(); Level++)
						{
							glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, Level, InternalFormat, Width >> Level, Height >> Level, 0, InImage.GetSize(Level), InImage.GetCubeData(Face, Level));
						}
					}

					MipmapsCount = InImage.GetMipmapsCount();

					break;
				}

				case Image::Type::Image2DArray: break;
			}
		}
		else
		{
			switch (InImage.GetType())
			{
				case Image::Type::Image2D:
				{
					glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
					glTexImage2D(Target, 0, InternalFormat, Width, Height, 0, PixelFormat, PixelType, InImage.Get2DData(0));
					glGenerateMipmap(Target);
					MipmapsCount = 1 + floor(log2(Math::Max(Width, Height)));

					break;
				}

				case Image::Type::Image3D: break;

				case Image::Type::ImageCube:
				{
					glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

					for (uint32 Face = 0; Face < 6; Face++)
					{
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, 0, InternalFormat, Width, Height, 0, PixelFormat, PixelType, InImage.GetCubeData(Face, 0));
					}

					glGenerateMipmap(Target);
					MipmapsCount = 1 + floor(log2(Math::Max(Width, Height)));

					break;
				}

				case Image::Type::Image2DArray: break;
			}
		}

		TextureFlags.Wrapping = Texture::Wrap::Repeat;

		SetFlags(TextureFlags);

		glBindTexture(Target, 0);

		return Success;
	}

	bool TextureOpenGL::Load(const char* File)
	{
		Image TmpImage;

		if (!TmpImage.Load(File))
		{
			return false;
		}

		bool Result = Create(TmpImage.GetType(), TextureDesc(TmpImage.GetWidth(), TmpImage.GetHeight(), 0, 0, TmpImage.GetFormat()));

		if (Result)
		{
			return Load(TmpImage);
		}

		return false;
	}

	bool TextureOpenGL::Create(Image::Type InType, TextureDesc Desc)
	{
		switch (InType)
		{
		case Image::Type::Image2D: return Create2D(Desc); break;
		case Image::Type::Image3D: break;
		case Image::Type::ImageCube: return CreateCube(Desc); break;
		case Image::Type::Image2DArray: break;
		}

		return false;
	}

	bool TextureOpenGL::Create(Texture::Type InType, TextureDesc Desc)
	{
		switch (InType)
		{
		case Texture::Type::Texture2D: return Create2D(Desc); break;
		case Texture::Type::Texture3D: break;
		case Texture::Type::TextureCube: return CreateCube(Desc); break;
		case Texture::Type::Texture2DArray: break;
		}

		return false;
	}
	
	bool TextureOpenGL::Create2D(TextureDesc Desc)
	{
		TextureType = Texture::Type::Texture2D;

		Target = GL_TEXTURE_2D;
		Width = Desc.Width;
		Height = Desc.Height;
		Format = Desc.Format;
		Multisampling = Desc.Multisampling;
		MipmapsCount = 1;

		bool Compressed = false;

		UpdateFormat(Format, Compressed);

		if (Multisampling != 0)
		{
			Target = GL_TEXTURE_2D_MULTISAMPLE;
		}

		glBindTexture(Target, ID);

		if (Multisampling == 0)
		{
			if (Compressed)
			{
				auto Block = GetBlockSizeFromFormat(Format);
				glCompressedTexImage2D(Target, 0, InternalFormat, Width, Height, 0, Width * Height * Block / 16, 0);
			} else
			{
				glTexImage2D(Target, 0, InternalFormat, Width, Height, 0, PixelFormat, PixelType, 0);
			}
		} else
		{
			glTexImage2DMultisample(Target, Multisampling, InternalFormat, Width, Height, GL_TRUE);
		}

		glBindTexture(Target, 0);

		return true;
	}

	bool TextureOpenGL::CreateCube(TextureDesc Desc)
	{
		TextureType = Texture::Type::TextureCube;

		Target = GL_TEXTURE_CUBE_MAP;
		Width = Desc.Width;
		Height = Desc.Height;
		Format = Desc.Format;
		MipmapsCount = 1;

		bool Compressed = false;

		UpdateFormat(Format, Compressed);

		glBindTexture(Target, ID);

		if (Compressed)
		{
			auto Block = GetBlockSizeFromFormat(Format);

			for (uint32 Face = 0; Face < 6; Face++)
			{
				glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, 0, InternalFormat, Width, Height, 0, Width * Height * Block / 16, 0);
			}
		}
		else
		{
			for (uint32 Face = 0; Face < 6; Face++)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, 0, InternalFormat, Width, Height, 0, PixelFormat, PixelType, 0);
			}
		}

		glBindTexture(Target, 0);

		return true;
	}

	void TextureOpenGL::SetFlags(Texture::Flags F)
	{
		TextureFlags = F;

		if (Multisampling == 0)
		{
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

			GLuint ClampMode = GL_CLAMP_TO_EDGE;

			switch (TextureFlags.Wrapping)
			{
			case Texture::Wrap::Clamp:          ClampMode = GL_CLAMP_TO_EDGE;   break;
			case Texture::Wrap::Repeat:         ClampMode = GL_REPEAT;          break;
			case Texture::Wrap::MirroredRepeat: ClampMode = GL_MIRRORED_REPEAT; break;
			}

			glTexParameteri(Target, GL_TEXTURE_WRAP_S, ClampMode);
			glTexParameteri(Target, GL_TEXTURE_WRAP_T, ClampMode);
			glTexParameteri(Target, GL_TEXTURE_WRAP_R, ClampMode);

			glTexParameteri(Target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(Target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

			glBindTexture(Target, 0);
		}
	}

	void TextureOpenGL::SetMipmapLevel(uint32 Level, uint32 MaxLevel)
	{
		MipmapLevel = Level;

		glBindTexture(Target, ID);
		glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, Level);
		glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, MaxLevel);
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
	
	void TextureOpenGL::GenerateMipmap()
	{
		if (Multisampling == 0)
		{
			glBindTexture(Target, ID);
			glGenerateMipmap(Target);
			glBindTexture(Target, 0);

			MipmapsCount = 1 + floor(log2(Math::Max(Width, Height)));
		}
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


