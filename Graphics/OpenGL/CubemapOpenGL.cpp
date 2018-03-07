#include <Graphics/OpenGL/CubemapOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	C_CubemapOpenGL::C_CubemapOpenGL()
	{
		glGenTextures(1, &mID);
		mInited = true;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_CubemapOpenGL::C_CubemapOpenGL(std::array<std::string, 6> aPath)
	{
		glGenTextures(1, &mID);
		if (!load(aPath)) return;
		mInited = true;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_CubemapOpenGL::C_CubemapOpenGL(std::string aPath)
	{
		glGenTextures(1, &mID);
		if (!load(aPath)) return;
		mInited = true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_CubemapOpenGL::bind() const
	{
		if (mInited)
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_CubemapOpenGL::samplerCube(int i) const
	{
		if (mInited)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_CubemapOpenGL::unbind() const
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool C_CubemapOpenGL::load(std::array<std::string, 6> aPath)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, mID);

		for (int i = 0; i < 6; i++)
		{
			mBitmaps[i].load(aPath[i]);
			mBitmaps[i].flipY();

			if (!mBitmaps[i].isExist())
			{
				C_Log::error("Can't load Cubemap");
				glDeleteTextures(1, &mID);
				return false;
			}
			else
			{
				unsigned int format = GL_RGBA;
				if (mBitmaps[i].getBPP() == 3) format = GL_RGB;

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
					mBitmaps[i].getWidth(), mBitmaps[i].getHeight(), 0, format, GL_UNSIGNED_BYTE,
						mBitmaps[i].getData());
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_CubemapOpenGL::load(std::string aPath)
	{
		std::array<std::string, 6> pathes;
		std::string names[6] =  { "PosX", "NegX", "PosY", "NegY", "PosZ", "NegZ" };

		Serializer::C_SerializerXML serializer;

		if (!serializer.read(aPath, "Cubemap"))
		{ C_Log::error("Can't load Cubemap XML: " + aPath); return false; }

		for (int i = 0; i < 6; i++)
		{
			if (!serializer.getString(names[i], &pathes[i]))
			{ C_Log::error("Can't load Cubemap XML: " + aPath); return false; }
		}

		return load(pathes);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	unsigned int C_CubemapOpenGL::getID() const
	{
		return mID;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_CubemapOpenGL::~C_CubemapOpenGL()
	{
		glDeleteTextures(1, &mID);
	}

}











