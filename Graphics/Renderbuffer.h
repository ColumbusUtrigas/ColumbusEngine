/************************************************
*                 Renderbuffer.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   05.10.2017                  *
*************************************************/

#pragma once

#include <GL/glew.h>

#include <System/System.h>
#include <System/Console.h>

namespace C
{

  enum C_RENDERBUFFER_FORMAT
  {
    C_RENDERBUFFER_DEPTH_16,
    C_RENDERBUFFER_DEPTH_24,
    C_RENDERBUFFER_DEPTH_32,
    C_RENDERBUFFER_DEPTH_24_STENCIL_8
  };

  class C_Renderbuffer
  {
  private:
    unsigned int mID;
  public:
    C_Renderbuffer();

    void bind();

    void storage(C_RENDERBUFFER_FORMAT aFormat, const int aWidth, const int aHeight);

    static void unbind();

    unsigned int getID();

    ~C_Renderbuffer();
  };

}
