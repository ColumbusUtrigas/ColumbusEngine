/************************************************
*                  Framebuffer.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   05.10.2017                  *
*************************************************/

#pragma once

#include <GL/glew.h>

#include <Graphics/Texture.h>
#include <Graphics/Renderbuffer.h>

#include <System/System.h>
#include <System/Console.h>

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

  enum C_FRAMEBUFFER_ATTACHMENT
  {
    C_FRAMEBUFFER_COLOR_ATTACH,
    C_FRAMEBUFFER_DEPTH_ATTACH,
    C_FRAMEBUFFER_STENCIL_ATTACH
  };

  class C_Framebuffer
  {
  private:
    unsigned int mID;
  public:
    C_Framebuffer();

    void bind();

    void setTexture2D(C_FRAMEBUFFER_ATTACHMENT aAttach, int aID);

    void setRenderbuffer(C_FRAMEBUFFER_ATTACHMENT aAttach, int aID);

    static void unbind();

    void prepare(C_Vector4 aClear, C_Vector2 aWindowSize);

    bool check();

    ~C_Framebuffer();
  };

}
