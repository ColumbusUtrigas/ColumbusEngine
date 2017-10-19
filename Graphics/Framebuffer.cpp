/************************************************
*                 Framebuffer.cpp               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   05.10.2017                  *
*************************************************/

#include <Graphics/Framebuffer.h>

namespace C
{

  C_Framebuffer::C_Framebuffer()
  {
    glGenFramebuffers(1, &mID);
  }

  void C_Framebuffer::bind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, mID);
  }

  void C_Framebuffer::setTexture2D(C_FRAMEBUFFER_ATTACHMENT aAttach, int aID)
  {
    if (!glIsTexture(aID))
    {
      printf("It's not a texture\n");
      return;
    }

    unsigned int attach = GL_COLOR_ATTACHMENT0;

    switch (aAttach)
    {
      case C_FRAMEBUFFER_COLOR_ATTACH:
        attach = GL_COLOR_ATTACHMENT0;
        break;
      case C_FRAMEBUFFER_DEPTH_ATTACH:
        attach = GL_DEPTH_ATTACHMENT;
        break;
      case C_FRAMEBUFFER_STENCIL_ATTACH:
        attach = GL_STENCIL_ATTACHMENT;
        break;
    }

    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, attach, GL_TEXTURE_2D, aID, 0);
    unbind();
  }

  void C_Framebuffer::setRenderbuffer(C_FRAMEBUFFER_ATTACHMENT aAttach, int aID)
  {
    if (!glIsRenderbuffer(aID))
    {
      printf("It's not a renderbuffer\n");
      return;
    }

    unsigned int attach = GL_DEPTH_ATTACHMENT;

    switch (aAttach)
    {
      case C_FRAMEBUFFER_COLOR_ATTACH:
        attach = GL_COLOR_ATTACHMENT0;
        break;
      case C_FRAMEBUFFER_DEPTH_ATTACH:
        attach = GL_DEPTH_ATTACHMENT;
        break;
      case C_FRAMEBUFFER_STENCIL_ATTACH:
        attach = GL_STENCIL_ATTACHMENT;
        break;
    }

    bind();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attach, GL_RENDERBUFFER, aID);
    unbind();
  }

  void C_Framebuffer::unbind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void C_Framebuffer::prepare(C_Vector4 aClear, C_Vector2 aWindowSize)
  {
    bind();
    C_ViewportOpenGL(0, 0, aWindowSize.x, aWindowSize.y);
		C_ClearColorOpenGL(aClear.x, aClear.y, aClear.z, aClear.w);
		C_ClearOpenGL(C_OGL_COLOR_BUFFER_BIT | C_OGL_DEPTH_BUFFER_BIT);
		C_EnableDepthTestOpenGL();
  }

  bool C_Framebuffer::check()
  {
    bind();
    bool ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    unbind();

    return ret;
  }

  C_Framebuffer::~C_Framebuffer()
  {
    glDeleteFramebuffers(1, &mID);
  }

}
