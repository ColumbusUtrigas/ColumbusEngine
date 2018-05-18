#include <Graphics/OpenGL/TextureOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	TextureOpenGL::TextureOpenGL()
	{
		glGenTextures(1, &mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	TextureOpenGL::TextureOpenGL(std::string aPath, bool aSmooth)
	{
		glGenTextures(1, &mID);
		load(aPath, aSmooth);
	}
	//////////////////////////////////////////////////////////////////////////////
	TextureOpenGL::TextureOpenGL(const char* aData, const int aW, const int aH, bool aSmooth)
	{
		glGenTextures(1, &mID);
		load(aData, aW, aH, aSmooth);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void TextureOpenGL::load(std::string aPath, bool aSmooth)
	{
		mImage.Load(aPath);

		glBindTexture(GL_TEXTURE_2D, mID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		uint32 OpenGLFormat = GL_RGBA;
		bool IsDXT = false;
		
		switch (mImage.GetFormat())
		{
		case TextureFormat::RGB:  OpenGLFormat = GL_RGB;  break;
		case TextureFormat::BGR:  OpenGLFormat = GL_BGR;  break;
		case TextureFormat::RGBA: OpenGLFormat = GL_RGBA; break;
		case TextureFormat::BGRA: OpenGLFormat = GL_BGRA; break;
		case TextureFormat::S3TC_A1:
			OpenGLFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			IsDXT = true;
			break;
		case TextureFormat::S3TC_A4:
			OpenGLFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			IsDXT = true;
			break;
		case TextureFormat::S3TC_A8:
			OpenGLFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			IsDXT = true;
			break;
		default: OpenGLFormat = GL_RGBA; break;
		}

		if (IsDXT)
		{
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, OpenGLFormat, mImage.GetWidth(), mImage.GetHeight(), 0, mImage.GetSize(), mImage.GetData());
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, OpenGLFormat, mImage.GetWidth(), mImage.GetHeight(), 0, OpenGLFormat, GL_UNSIGNED_BYTE, mImage.GetData());
		}

		if (mConfig.mipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, mConfig.LOD);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 9);
		}

		unsigned int filter;

		if (mConfig.smooth)
		{
			mConfig.mipmaps ? (filter = GL_LINEAR_MIPMAP_LINEAR) : (filter = GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			mConfig.mipmaps ? (filter = GL_NEAREST_MIPMAP_NEAREST) : (filter = GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, mConfig.anisotropy);

		glBindTexture(GL_TEXTURE_2D, 0);

		Log::success("Texture loaded: " + aPath);
	}

	//////////////////////////////////////////////////////////////////////////////
	void TextureOpenGL::load(const char* aData, const int aW, const int aH, bool aSmooth)
	{
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glBindTexture(GL_TEXTURE_2D, mID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		unsigned int filter;

		if (mConfig.smooth)
		{
			mConfig.mipmaps ? (filter = GL_LINEAR_MIPMAP_LINEAR) : (filter = GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			mConfig.mipmaps ? (filter = GL_NEAREST_MIPMAP_NEAREST) : (filter = GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA/*GL_RED*/, aW, aH,
		//0, GL_RGBA/*GL_RED*/, GL_UNSIGNED_BYTE, (void*)aData);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, aW, aH, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, (void*)aData);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	void TextureOpenGL::loadDepth(const char* aData, const int aW, const int aH, bool aSmooth)
	{
		glBindTexture(GL_TEXTURE_2D, mID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		unsigned int filter;

		if (mConfig.smooth)
		{
			mConfig.mipmaps ? (filter = GL_LINEAR_MIPMAP_LINEAR) : (filter = GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			mConfig.mipmaps ? (filter = GL_NEAREST_MIPMAP_NEAREST) : (filter = GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, aW, aH, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void TextureOpenGL::setConfig(TextureConfig aConfig)
	{
		TextureConfig conf = mConfig;

		mConfig = aConfig;

		if (aConfig.smooth != conf.smooth) setSmooth(aConfig.smooth);
		if (aConfig.anisotropy != conf.anisotropy) setAnisotropy(aConfig.anisotropy);
	}
	//////////////////////////////////////////////////////////////////////////////
	void TextureOpenGL::setSmooth(const bool aSmooth)
	{
		if (mConfig.smooth != aSmooth)
		{
			unsigned int filter;

			if (aSmooth == true)
				mConfig.mipmaps ? (filter = GL_LINEAR_MIPMAP_LINEAR) : (filter = GL_LINEAR);
			else
				mConfig.mipmaps ? (filter = GL_NEAREST_MIPMAP_NEAREST) : (filter = GL_NEAREST);

			mConfig.smooth = (bool)aSmooth;
			bind();
			glTexParameteri(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(C_OGL_TEXTURE_2D, C_OGL_TEXTURE_MAG_FILTER, filter);
			unbind();
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void TextureOpenGL::setAnisotropy(const unsigned int aAnisotropy)
	{
		if (mConfig.anisotropy != aAnisotropy)
		{
			mConfig.anisotropy = (unsigned int)aAnisotropy;
			bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aAnisotropy);
			unbind();
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void TextureOpenGL::bind()
	{
		if(mID != 0) glBindTexture(GL_TEXTURE_2D, mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	void TextureOpenGL::unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void TextureOpenGL::sampler2D(int a)
	{
		glActiveTexture(GL_TEXTURE0 + a);
		bind();
	}
	//////////////////////////////////////////////////////////////////////////////
	void TextureOpenGL::generateMipmap()
	{
		bind();
		glGenerateMipmap(GL_TEXTURE_2D);
		unbind();
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	std::string TextureOpenGL::getType()
	{
		return "OpenGL Texture";
	}
	//////////////////////////////////////////////////////////////////////////////
	unsigned int TextureOpenGL::getID() const
	{
		return mID;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	TextureOpenGL::~TextureOpenGL()
	{
		glDeleteTextures(1, &mID);
	}

}









