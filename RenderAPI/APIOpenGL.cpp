/************************************************
*                 APIOpenGL.cpp                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                  10.10.2017                   *
*************************************************/

#include <RenderAPI/APIOpenGL.h>

namespace C
{

  //////////////////////////////////////////////////////////////////////////////
  //Get OpenGL error
  bool C_GetErrorOpenGL()
	{
		GLenum err;
		if (err != GL_NO_ERROR)
		{
			std::string error;

			switch (err)
			{
  			case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
  			case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
  			case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
  			case GL_STACK_OVERFLOW:         error = "STACK_OVERFLOW";         break;
  			case GL_STACK_UNDERFLOW:        error = "STACK_UNDERFLOW";        break;
  			case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
  			case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
  			case GL_TABLE_TOO_LARGE:        error = "TABLE_TOO_LARGE";        break;
			}

			C_Error("OpenGL: %s\n", error.c_str());
			return true;
		}
		return false;
	}
  //////////////////////////////////////////////////////////////////////////////
  //Generate OpenGL GPU buffer
	void C_GenBufferOpenGL(unsigned int* aID)
	{
		glGenBuffers(1, aID);
	}
  //////////////////////////////////////////////////////////////////////////////
  //Delete OpenGL GPU buffer
	void C_DeleteBufferOpenGL(unsigned int* aID)
	{
		glDeleteBuffers(1, aID);
	}
  //////////////////////////////////////////////////////////////////////////////
  //Bind OpenGL GPU buffer
  void C_BindBufferOpenGL(unsigned int aTarget, unsigned int aID)
  {
    glBindBuffer(aTarget, aID);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Load OpenGL buffer data to GPU
  void C_BufferDataOpenGL(unsigned int aTarget, size_t aSize,
                          const void* aData, unsigned int aUsage)
  {
    glBufferData(aTarget, aSize, aData, aUsage);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Generate OpenGL texture
  void C_GenTextureOpenGL(unsigned int* aID)
  {
    glGenTextures(1, aID);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Delete OpenGL texture
  void C_DeleteTextureOpenGL(unsigned int* aID)
  {
    glDeleteTextures(1, aID);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Bind OpenGL texture
  void C_BindTextureOpenGL(unsigned int aTarget, unsigned int aID)
  {
    glBindTexture(aTarget, aID);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Load OpenGL texture data to GPU
  void C_Texture2DOpenGL(unsigned int aTarget, unsigned int aLevel,
    unsigned int aInternalFormat, size_t aWidth, size_t aHeight,
    unsigned int aFormat, unsigned int aType, const void* aData)
  {

    glTexImage2D(aTarget, 0, aInternalFormat, aWidth, aHeight, 0, aFormat, aType, aData);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set OpenGL texture parameters
  void C_TextureParameter(unsigned int aTarget, int aParam, unsigned int aVal)
  {
    glTexParameteri(aTarget, aParam, aVal);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Generate mip levels from GPU texture
  void C_GenMipmapOpenGL(unsigned int aTarget)
  {
    glGenerateMipmap(aTarget);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Activate GPU texture
  void C_ActiveTextureOpenGL(unsigned int aTexture)
  {
    glActiveTexture(aTexture);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Open vertex attribute stream
  void C_OpenStreamOpenGL(unsigned int aStream)
  {
    glEnableVertexAttribArray(aStream);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Close vertex attribute stream
  void C_CloseStreamOpenGL(unsigned int aStream)
  {
    glDisableVertexAttribArray(aStream);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Enable writing into the depth buffer
  void C_EnableDepthMask()
  {
    glDepthMask(GL_TRUE);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Disable writing into the depth buffer
  void C_DisableDepthMask()
  {
    glDepthMask(GL_FALSE);
  }

}
