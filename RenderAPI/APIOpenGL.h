/************************************************
*                  APIOpenGL.h                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                  10.10.2017                   *
*************************************************/

#pragma once

#include <GL/glew.h>
#include <string>

#include <System/Console.h>

namespace C
{

  #define C_OGL_ARRAY_BUFFER GL_ARRAY_BUFFER

  #define C_OGL_STATIC_DRAW    GL_STATIC_DRAW
  #define C_OGL_DYNAMIC_DRAW   GL_DYNAMIC_DRAW
  #define C_OGL_STREAM_DRAW    GL_STREAM_DRAW

  #define C_OGL_TEXTURE_2D               GL_TEXTURE_2D
  #define C_OGL_TEXTURE_CUBE_MAP         GL_TEXTURE_CUBE_MAP
  #define C_OGL_TEXTURE_CUBE_MAP_POS_X   GL_TEXTURE_CUBE_MAP_POSITIVE_X
  #define C_OGL_TEXTURE_CUBE_MAP_NEG_X   GL_TEXTURE_CUBE_MAP_NEGATIVE_X
  #define C_OGL_TEXTURE_CUBE_MAP_POS_Y   GL_TEXTURE_CUBE_MAP_POSITIVE_Y
  #define C_OGL_TEXTURE_CUBE_MAP_NEG_Y   GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
  #define C_OGL_TEXTURE_CUBE_MAP_POS_Z   GL_TEXTURE_CUBE_MAP_POSITIVE_Z
  #define C_OGL_TEXTURE_CUBE_MAP_NEG_Z   GL_TEXTURE_CUBE_MAP_NEGATIVE_Z

  #define C_OGL_TEXTURE_BASE_LEVEL   GL_TEXTURE_BASE_LEVEL
  #define C_OGL_TEXTURE_MIN_FILTER   GL_TEXTURE_MIN_FILTER
  #define C_OGL_TEXTURE_MAG_FILTER   GL_TEXTURE_MAG_FILTER
  #define C_OGL_TEXTURE_MAX_LEVEL    GL_TEXTURE_MAX_LEVEL
  #define C_OGL_TEXTURE_WRAP_S       GL_TEXTURE_WRAP_S
  #define C_OGL_TEXTURE_WRAP_T       GL_TEXTURE_WRAP_T
  #define C_OGL_TEXTURE_WRAP_R       GL_TEXTURE_WRAP_R

  #define C_OGL_NEAREST                  GL_NEAREST
  #define C_OGL_LINEAR                   GL_LINEAR
  #define C_OGL_NEAREST_MIPMAP_NEAREST   GL_NEAREST_MIPMAP_NEAREST
  #define C_OGL_LINEAR_MIPMAP_NEAREST    GL_LINEAR_MIPMAP_NEAREST
  #define C_OGL_NEAREST_MIPMAP_LINEAR    GL_NEAREST_MIPMAP_LINEAR
  #define C_OGL_LINEAR_MIPMAP_LINEAR     GL_LINEAR_MIPMAP_LINEAR
  #define C_OGL_CLAMP_TO_EDGE            GL_CLAMP_TO_EDGE
  #define C_OGL_CLAMP_TO_BORDER          GL_CLAMP_TO_BORDER
  #define C_OGL_MIRRORED_REPEAT          GL_MIRRORED_REPEAT
  #define C_OGL_REPEAT                   GL_REPEAT
  #define C_OGL_MIRROR_CLAMP_TO_EDGE     GL_MIRROR_CLAMP_TO_EDGE

  #define C_OGL_RED               GL_RED
  #define C_OGL_RG                GL_RG
  #define C_OGL_RGB               GL_RGB
  #define C_OGL_BGR               GL_BGR
  #define C_OGL_RGBA              GL_RGBA
  #define C_OGL_BGRA              GL_BGRA
  #define C_OGL_RED_INTEGER       GL_RED_INTEGER
  #define C_OGL_RG_INTEGER        GL_RG_INTEGER
  #define C_OGL_RGB_INTEGER       GL_RGB_INTEGER
  #define C_OGL_BGR_INTEGER       GL_BGR_INTEGER
  #define C_OGL_RGBA_INTEGER      GL_RGBA_INTEGER
  #define C_OGL_BGRA_INTEGER      GL_BGRA_INTEGER
  #define C_OGL_STENCIL_INDEX     GL_STENCIL_INDEX
  #define C_OGL_DEPTH_COMPONENT   GL_DEPTH_COMPONENT
  #define C_OGL_DEPTH_STENCIL     GL_DEPTH_STENCIL

  #define C_OGL_UNSIGNED_BYTE                 GL_UNSIGNED_BYTE
  #define C_OGL_BYTE                          GL__BYTE
  #define C_OGL_UNSIGNED_SHORT                GL_UNSIGNED_SHORT
  #define C_OGL_SHORT                         GL_SHORT
  #define C_OGL_UNSIGNED_INT                  GL_UNSIGNED_INT
  #define C_OGL_INT                           GL_INT
  #define C_OGL_FLOAT                         GL_FLOAT
  #define C_OGL_UNSIGNED_BYTE_3_3_2           GL_UNSIGNED_BYTE_3_3_2
  #define C_OGL_UNSIGNED_BYTE_2_3_3_REV       GL_UNSIGNED_BYTE_2_3_3_REV
  #define C_OGL_UNSIGNED_SHORT_5_6_5          GL_UNSIGNED_SHORT_5_6_5
  #define C_OGL_UNSIGNED_SHORT_5_6_5_REV      GL_UNSIGNED_SHORT_5_6_5_REV
  #define C_OGL_UNSIGNED_SHORT_4_4_4_4        GL_UNSIGNED_SHORT_4_4_4_4
  #define C_OGL_UNSIGNED_SHORT_4_4_4_4_REV    GL_UNSIGNED_SHORT_4_4_4_4_REV
  #define C_OGL_UNSIGNED_SHORT_5_5_5_1        GL_UNSIGNED_SHORT_5_5_5_1
  #define C_OGL_UNSIGNED_SHORT_1_5_5_5_REV    GL_UNSIGNED_SHORT_1_5_5_5_REV
  #define C_OGL_UNSIGNED_INT_8_8_8_8          GL_UNSIGNED_INT_8_8_8_8
  #define C_OGL_UNSIGNED_INT_8_8_8_8_REV      GL_UNSIGNED_INT_8_8_8_8_REV
  #define C_OGL_UNSIGNED_INT_10_10_10_2       GL_UNSIGNED_INT_10_10_10_2
  #define C_OGL_UNSIGNED_INT_2_10_10_10_REV   GL_UNSIGNED_INT_2_10_10_10_REV

  //Get OpenGL error
  bool C_GetErrorOpenGL();
  //Generate OpenGL GPU buffer
  void C_GenBufferOpenGL(unsigned int* aID);
  //Delete OpenGL GPU buffer
  void C_DeleteBufferOpenGL(unsigned int* aID);
  //Bind OpenGL GPU buffer
  void C_BindBufferOpenGL(unsigned int aTarget, unsigned int aID);
  //Load OpenGL buffer data to GPU
  void C_BufferDataOpenGL(unsigned int aTarget, size_t aSize,
                          const void* aData, unsigned int aUsage);
  //Generate OpenGL texture
  void C_GenTextureOpenGL(unsigned int* aID);
  //Delete OpenGL texture
  void C_DeleteTextureOpenGL(unsigned int* aID);
  //Bind OpenGL texture
  void C_BindTextureOpenGL(unsigned int aTarget, unsigned int aID);
  //Load OpenGL texture data to GPU
  void C_Texture2DOpenGL(unsigned int aTarget, unsigned int aLevel,
    unsigned int aInternalFormat, size_t aWidth, size_t aHeight,
    unsigned int aFormat, unsigned int aType, const void* aData);
  //Set OpenGL texture parameters
  void C_TextureParameter(unsigned int aTarget, int aParam, unsigned int aVal);

}
