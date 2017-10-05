/************************************************
*                Renderbuffer.cpp               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   05.10.2017                  *
*************************************************/

#include <Graphics/Renderbuffer.h>

namespace C
{

  C_Renderbuffer::C_Renderbuffer()
  {
    glGenRenderbuffers(1, &mID);
  }

  void C_Renderbuffer::bind()
  {
    glBindRenderbuffer(GL_RENDERBUFFER, mID);
  }

  void C_Renderbuffer::storage(C_RENDERBUFFER_FORMAT aFormat, const int aWidth, const int aHeight)
  {
    unsigned int format = GL_DEPTH_COMPONENT24;

    switch (aFormat)
    {
      case C_RENDERBUFFER_DEPTH_16:
        format = GL_DEPTH_COMPONENT16;
        break;
      case C_RENDERBUFFER_DEPTH_24:
        format = GL_DEPTH_COMPONENT24;
        break;
      case C_RENDERBUFFER_DEPTH_32:
        format = GL_DEPTH_COMPONENT32;
        break;
      case C_RENDERBUFFER_DEPTH_24_STENCIL_8:
        format = GL_DEPTH24_STENCIL8;
        break;
    }
    bind();
    glRenderbufferStorage(GL_RENDERBUFFER, format, aWidth, aHeight);
    unbind();
  }

  void C_Renderbuffer::unbind()
  {
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  }

  unsigned int C_Renderbuffer::getID()
  {
    return mID;
  }

  C_Renderbuffer::~C_Renderbuffer()
  {
    glDeleteRenderbuffers(1, &mID);
  }

}
