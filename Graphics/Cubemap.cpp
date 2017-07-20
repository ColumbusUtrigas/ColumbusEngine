#include <Graphics/Cubemap.h>

namespace C
{

	C_Cubemap::C_Cubemap(C_CubemapPath aPath)
	{
		glGenTextures(1, &mID);

		glBindTexture(GL_TEXTURE_CUBE_MAP, mID);

		char* data[6];
		int nWidth[6];
		int nHeight[6];

		data[0] = C_LoadImage(aPath[0].c_str(), &nWidth[0], &nHeight[0]);
		data[1] = C_LoadImage(aPath[1].c_str(), &nWidth[1], &nHeight[1]);
		data[2] = C_LoadImage(aPath[2].c_str(), &nWidth[2], &nHeight[2]);
		data[3] = C_LoadImage(aPath[3].c_str(), &nWidth[3], &nHeight[3]);
		data[4] = C_LoadImage(aPath[4].c_str(), &nWidth[4], &nHeight[4]);
		data[5] = C_LoadImage(aPath[5].c_str(), &nWidth[5], &nHeight[5]);

		if (data[0] == NULL) { printf("Error: Can't load Cubemap"); glDeleteTextures(1, &mID); return; }
		if (data[1] == NULL) { printf("Error: Can't load Cubemap"); glDeleteTextures(1, &mID); return; }
		if (data[2] == NULL) { printf("Error: Can't load Cubemap"); glDeleteTextures(1, &mID); return; }
		if (data[3] == NULL) { printf("Error: Can't load Cubemap"); glDeleteTextures(1, &mID); return; }
		if (data[4] == NULL) { printf("Error: Can't load Cubemap"); glDeleteTextures(1, &mID); return; }
		if (data[5] == NULL) { printf("Error: Can't load Cubemap"); glDeleteTextures(1, &mID); return; }

		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, nWidth[0], nHeight[0], 0, GL_BGRA, GL_UNSIGNED_BYTE, data[0]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, nWidth[1], nHeight[1], 0, GL_BGRA, GL_UNSIGNED_BYTE, data[1]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, nWidth[2], nHeight[2], 0, GL_BGRA, GL_UNSIGNED_BYTE, data[2]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, nWidth[3], nHeight[3], 0, GL_BGRA, GL_UNSIGNED_BYTE, data[3]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, nWidth[4], nHeight[4], 0, GL_BGRA, GL_UNSIGNED_BYTE, data[4]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, nWidth[5], nHeight[5], 0, GL_BGRA, GL_UNSIGNED_BYTE, data[5]);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	void C_Cubemap::bind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
	}

	void C_Cubemap::samplerCube(int i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		bind();
	}

	void C_Cubemap::unbind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	C_Cubemap::~C_Cubemap()
	{

	}

}



