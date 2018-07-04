#include <Graphics/OpenGL/CubemapOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	CubemapOpenGL::CubemapOpenGL()
	{
		glGenTextures(1, &mID);
		mInited = true;
	}
	//////////////////////////////////////////////////////////////////////////////
	CubemapOpenGL::CubemapOpenGL(std::array<std::string, 6> aPath)
	{
		glGenTextures(1, &mID);
		if (!load(aPath)) return;
		mInited = true;
	}
	//////////////////////////////////////////////////////////////////////////////
	CubemapOpenGL::CubemapOpenGL(std::string aPath)
	{
		glGenTextures(1, &mID);
		if (!load(aPath)) return;
		mInited = true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void CubemapOpenGL::bind() const
	{
		if (mInited)
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void CubemapOpenGL::samplerCube(int i) const
	{
		if (mInited)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void CubemapOpenGL::unbind() const
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool CubemapOpenGL::load(std::array<std::string, 6> aPath)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, mID);

		for (int i = 0; i < 6; i++)
		{
			if (!mBitmaps[i].Load(aPath[i], ImageLoading::FlipY))
			{
				Log::error("Can't load Cubemap face: " + aPath[i]);
				glDeleteTextures(1, &mID);
				return false;
			}

			if (!mBitmaps[i].IsExist())
			{
				Log::error("Can't load Cubemap");
				glDeleteTextures(1, &mID);
				return false;
			}
			else
			{
				uint32 format = GL_RGBA;
				bool IsDXT = false;

				switch (mBitmaps[i].GetFormat())
				{
				case TextureFormat::RGB8:  format = GL_RGB;  break;
				case TextureFormat::RGBA8: format = GL_RGBA; break;
				case TextureFormat::DXT1: format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; IsDXT = true; break;
				case TextureFormat::DXT3: format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; IsDXT = true; break;
				case TextureFormat::DXT5: format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; IsDXT = true; break;
				default: format = GL_RGBA; break;
				}

				if (IsDXT)
				{
					glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
						mBitmaps[i].GetWidth(), mBitmaps[i].GetHeight(), 0, mBitmaps[i].GetSize(0),
						mBitmaps[i].GetData());
				}
				else
				{
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
						mBitmaps[i].GetWidth(), mBitmaps[i].GetHeight(), 0, format, GL_UNSIGNED_BYTE,
						mBitmaps[i].GetData());
				}
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		for (uint32 i = 0; i < 6; i++)
		{
			mBitmaps[i].FreeData(); //Hmmm
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool CubemapOpenGL::load(std::string aPath)
	{
		std::array<std::string, 6> pathes;
		std::string names[6] =  { "PosX", "NegX", "PosY", "NegY", "PosZ", "NegZ" };

		Serializer::SerializerXML Serializer;

		if (!Serializer.Read(aPath, "Cubemap"))
		{ Log::error("Can't load Cubemap XML: " + aPath); return false; }

		for (int i = 0; i < 6; i++)
		{
			if (!Serializer.GetString(names[i], pathes[i]))
			{ Log::error("Can't load Cubemap XML: " + aPath); return false; }
		}

		return load(pathes);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	unsigned int CubemapOpenGL::getID() const
	{
		return mID;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	CubemapOpenGL::~CubemapOpenGL()
	{
		glDeleteTextures(1, &mID);
	}

}











