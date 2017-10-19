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
  void C_TextureParameterOpenGL(unsigned int aTarget, int aParam, unsigned int aVal)
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
  //Deactivate GPU texture
  void C_DeactiveTextureOpenGL(unsigned int aTexture)
  {
    glActiveTexture(aTexture);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Deactivate GPU cube map texture
  void C_DeactiveCubemapOpenGL(unsigned int aCubemap)
  {
    glActiveTexture(aCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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
  void C_EnableDepthMaskOpenGL()
  {
    glDepthMask(GL_TRUE);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Disable writing into the depth buffer
  void C_DisableDepthMaskOpenGL()
  {
    glDepthMask(GL_FALSE);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set OpenGL buffer as vertex attribute
  void C_VertexAttribPointerOpenGL(unsigned int aIndex, unsigned int aSize,
    unsigned int aType, unsigned int aNormalized, size_t aStride, const void* aData)
  {
    glVertexAttribPointer(aIndex, aSize, aType, aNormalized, aStride, aData);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Render primitive from array data
  void C_DrawArraysOpenGL(unsigned int aMode, int aFirst, size_t aCount)
  {
    glDrawArrays(aMode, aFirst, aCount);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Change pixel arithmetic
  void C_BlendFuncOpenGL(unsigned int aSFactor, unsigned int aDFactor)
  {
    glBlendFunc(aSFactor, aDFactor);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Enable depth testing
  void C_EnableDepthTestOpenGL()
  {
    glEnable(GL_DEPTH_TEST);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Disable depth testing
  void C_DisableDepthTestOpenGL()
  {
    glDisable(GL_DEPTH_TEST);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Enable texturing
  void C_EnableTextureOpenGL()
  {
    glEnable(GL_TEXTURE_2D);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Disable texturing
  void C_DisableTextureOpenGL()
  {
    glDisable(GL_TEXTURE_2D);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Enable blending
  void C_EnableBlendOpenGL()
  {
    glEnable(GL_BLEND);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Disable blending
  void C_DisableBlendOpenGL()
  {
    glDisable(GL_BLEND);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Enable culling
  void C_EnableCullFaceOpenGL()
  {
    glEnable(GL_CULL_FACE);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Disable culling
  void C_DisableCullFaceOpenGL()
  {
    glDisable(GL_CULL_FACE);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Enable cubemapping
  void C_EnableCubemapOpenGL()
  {
    glEnable(GL_TEXTURE_CUBE_MAP_ARB);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Disable cubemapping
  void C_DisableCubemapOpenGL()
  {
    glDisable(GL_TEXTURE_CUBE_MAP_ARB);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Enable multisampling
  void C_EnableMultisampleOpenGL()
  {
    glEnable(GL_MULTISAMPLE);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Disable multisampling
  void C_DisableMultisampleOpenGL()
  {
    glDisable(GL_MULTISAMPLE);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Change depth buffer algorithm
  void C_DepthFuncOpenGL(unsigned int aFunc)
  {
    glDepthFunc(aFunc);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Change face culling
  void C_CullFaceOpenGL(unsigned int aMode)
  {
    glCullFace(aMode);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Get OpenGL vendor
  std::string C_GetVendorOpenGL()
  {
    return (const char*)glGetString(GL_VENDOR);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Get OpenGL renderer
  std::string C_GetRendererOpenGL()
  {
    return (const char*)glGetString(GL_RENDERER);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Get OpenGL renderer
  std::string C_GetVersionOpenGL()
  {
    return (const char*)glGetString(GL_VERSION);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Get OpenGL renderer
  std::string C_GetGLSLVersionOpenGL()
  {
    return (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Clear OpenGL buffer
  void C_ClearOpenGL(unsigned int aMask)
  {
    glClear(aMask);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set OpenGL clear color
  void C_ClearColorOpenGL(float aR, float aG, float aB, float aA)
  {
    glClearColor(aR, aG, aB, aA);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set OpenGL viewport
  void C_ViewportOpenGL(int aX, int aY, size_t aW, size_t aH)
  {
    glViewport(aX, aY, aW, aH);
  }

}
