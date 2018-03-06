#include <Graphics/OpenGL/TextureOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	C_TextureOpenGL::C_TextureOpenGL()
	{
		glGenTextures(1, &mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	C_TextureOpenGL::C_TextureOpenGL(std::string aPath, bool aSmooth)
	{
		glGenTextures(1, &mID);
		load(aPath, aSmooth);
	}
	//////////////////////////////////////////////////////////////////////////////
	C_TextureOpenGL::C_TextureOpenGL(const char* aData, const int aW, const int aH, bool aSmooth)
	{
		glGenTextures(1, &mID);
		load(aData, aW, aH, aSmooth);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_TextureOpenGL::load(std::string aPath, bool aSmooth)
	{
		mImage.load(aPath);

		glBindTexture(GL_TEXTURE_2D, mID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		unsigned int format = GL_RGBA;
		if (mImage.getBPP() == 3) format = GL_RGB;

		glTexImage2D(GL_TEXTURE_2D, 0, format, mImage.getWidth(), mImage.getHeight(), 0,
			format, GL_UNSIGNED_BYTE, mImage.getData());

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

		C_Log::success("Texture loaded: " + aPath);
	}

	//////////////////////////////////////////////////////////////////////////////
	void C_TextureOpenGL::load(const char* aData, const int aW, const int aH, bool aSmooth)
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
	void C_TextureOpenGL::loadDepth(const char* aData, const int aW, const int aH, bool aSmooth)
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
	void C_TextureOpenGL::setConfig(C_TextureConfig aConfig)
	{
		C_TextureConfig conf = mConfig;

		mConfig = aConfig;

		if (aConfig.smooth != conf.smooth) setSmooth(aConfig.smooth);
		if (aConfig.anisotropy != conf.anisotropy) setAnisotropy(aConfig.anisotropy);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_TextureOpenGL::setSmooth(const bool aSmooth)
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
	void C_TextureOpenGL::setAnisotropy(const unsigned int aAnisotropy)
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
	void C_TextureOpenGL::bind()
	{
		if(mID != 0) glBindTexture(GL_TEXTURE_2D, mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_TextureOpenGL::unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_TextureOpenGL::sampler2D(int a)
	{
		glActiveTexture(GL_TEXTURE0 + a);
		bind();
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_TextureOpenGL::generateMipmap()
	{
		bind();
		glGenerateMipmap(GL_TEXTURE_2D);
		unbind();
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	std::string C_TextureOpenGL::getType()
	{
		return "OpenGL Texture";
	}
	//////////////////////////////////////////////////////////////////////////////
	unsigned int C_TextureOpenGL::getID() const
	{
		return mID;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_TextureOpenGL::~C_TextureOpenGL()
	{
		glDeleteTextures(1, &mID);
	}

}








