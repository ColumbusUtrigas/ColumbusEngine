/************************************************
*                  APIOpenGL.h                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                  10.10.2017                   *
*************************************************/

#pragma once

#include <GL/glew.h>
#include <string>

#include <System/Log.h>

namespace Columbus
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

  #define C_OGL_TEXTURE_BASE_LEVEL       GL_TEXTURE_BASE_LEVEL
  #define C_OGL_TEXTURE_MIN_FILTER       GL_TEXTURE_MIN_FILTER
  #define C_OGL_TEXTURE_MAG_FILTER       GL_TEXTURE_MAG_FILTER
  #define C_OGL_TEXTURE_MAX_LEVEL        GL_TEXTURE_MAX_LEVEL
  #define C_OGL_TEXTURE_WRAP_S           GL_TEXTURE_WRAP_S
  #define C_OGL_TEXTURE_WRAP_T           GL_TEXTURE_WRAP_T
  #define C_OGL_TEXTURE_WRAP_R           GL_TEXTURE_WRAP_R
  #define C_OGL_TEXTURE_MAX_ANISOTROPY   GL_TEXTURE_MAX_ANISOTROPY_EXT

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

  #define C_OGL_TRUE    GL_TRUE
  #define C_OGL_FALSE   GL_FALSE

  #define C_OGL_TEXTURE0    GL_TEXTURE0
  #define C_OGL_TEXTURE1    GL_TEXTURE1
  #define C_OGL_TEXTURE2    GL_TEXTURE2
  #define C_OGL_TEXTURE3    GL_TEXTURE3
  #define C_OGL_TEXTURE4    GL_TEXTURE4
  #define C_OGL_TEXTURE5    GL_TEXTURE5
  #define C_OGL_TEXTURE6    GL_TEXTURE6
  #define C_OGL_TEXTURE7    GL_TEXTURE7
  #define C_OGL_TEXTURE8    GL_TEXTURE8
  #define C_OGL_TEXTURE9    GL_TEXTURE9
  #define C_OGL_TEXTURE10   GL_TEXTURE10
  #define C_OGL_TEXTURE11   GL_TEXTURE11
  #define C_OGL_TEXTURE12   GL_TEXTURE12
  #define C_OGL_TEXTURE13   GL_TEXTURE13
  #define C_OGL_TEXTURE14   GL_TEXTURE14
  #define C_OGL_TEXTURE15   GL_TEXTURE15
  #define C_OGL_TEXTURE16   GL_TEXTURE16
  #define C_OGL_TEXTURE17   GL_TEXTURE17
  #define C_OGL_TEXTURE18   GL_TEXTURE18
  #define C_OGL_TEXTURE19   GL_TEXTURE19
  #define C_OGL_TEXTURE20   GL_TEXTURE20
  #define C_OGL_TEXTURE21   GL_TEXTURE21
  #define C_OGL_TEXTURE22   GL_TEXTURE22
  #define C_OGL_TEXTURE23   GL_TEXTURE23
  #define C_OGL_TEXTURE24   GL_TEXTURE24
  #define C_OGL_TEXTURE25   GL_TEXTURE25
  #define C_OGL_TEXTURE26   GL_TEXTURE26
  #define C_OGL_TEXTURE27   GL_TEXTURE27
  #define C_OGL_TEXTURE28   GL_TEXTURE28
  #define C_OGL_TEXTURE29   GL_TEXTURE29
  #define C_OGL_TEXTURE30   GL_TEXTURE30
  #define C_OGL_TEXTURE31   GL_TEXTURE31

  #define C_OGL_POINTS                     GL_POINTS
  #define C_OGL_LINE_STRIP                 GL_LINE_STRIP
  #define C_OGL_LINE_LOOP                  GL_LINE_LOOP
  #define C_OGL_LINES                      GL_LINES
  #define C_OGL_LINE_STRIP_ADJACENCY       GL_LINE_STRIP_ADJACENCY
  #define C_OGL_LINES_ADJACENCY            GL_LINES_ADJACENCY
  #define C_OGL_TRIANGLE_STRIP             GL_TRIANGLE_STRIP
  #define C_OGL_TRIANGLE_FAN               GL_TRIANGLE_FAN
  #define C_OGL_TRIANGLES                  GL_TRIANGLES
  #define C_OGL_TRIANGLE_STRIP_ADJACENCY   GL_TRIANGLE_STRIP_ADJACENCY
  #define C_OGL_TRIANGLES_ADJACENCY        GL_TRIANGLES_ADJACENCY
  #define C_OGL_PATCHES                    GL_PATCHES

  #define C_OGL_ZERO                       GL_ZERO
  #define C_OGL_ONE                        GL_ONE
  #define C_OGL_SRC_COLOR                  GL_SRC_COLOR
  #define C_OGL_ONE_MINUS_SRC_COLOR        GL_ONE_MINUS_SRC_COLOR
  #define C_OGL_DST_COLOR                  GL_DST_COLOR
  #define C_OGL_ONE_MINUS_DST_COLOR        GL_ONE_MINUS_DST_COLOR
  #define C_OGL_SRC_ALPHA                  GL_SRC_ALPHA
  #define C_OGL_ONE_MINUS_SRC_ALPHA        GL_ONE_MINUS_SRC_ALPHA
  #define C_OGL_DST_ALPHA                  GL_DST_ALPHA
  #define C_OGL_ONE_MINUS_DST_ALPHA        GL_ONE_MINUS_DST_ALPHA
  #define C_OGL_CONSTANT_COLOR             GL_CONSTANT_COLOR
  #define C_OGL_ONE_MINUS_CONSTANT_COLOR   GL_ONE_MINUS_CONSTANT_COLOR
  #define C_OGL_CONSTANT_ALPHA             GL_CONSTANT_ALPHA
  #define C_OGL_ONE_MINUS_CONSTANT_ALPHA   GL_ONE_MINUS_CONSTANT_ALPHA

  #define C_OGL_NEVER      GL_NEVER
  #define C_OGL_LESS       GL_LESS
  #define C_OGL_LEQUAL     GL_LEQUAL
  #define C_OGL_EQUAL      GL_EQUAL
  #define C_OGL_GREATER    GL_GREATER
  #define C_OGL_NOTEQUAL   GL_NOTEQUAL
  #define C_OGL_GEQUAL     GL_GEQUAL
  #define C_OGL_ALWAYS     GL_ALWAYS

  #define C_OGL_FRONT            GL_FRONT
  #define C_OGL_BACK             GL_BACK
  #define C_OGL_FRONT_AND_BACK   GL_FRONT_AND_BACK

  #define C_OGL_COLOR_BUFFER_BIT     GL_COLOR_BUFFER_BIT
  #define C_OGL_DEPTH_BUFFER_BIT     GL_DEPTH_BUFFER_BIT
  #define C_OGL_ACCUM_BUFFER_BIT     GL_ACCUM_BUFFER_BIT
  #define C_OGL_STENCIL_BUFFER_BIT   GL_STENCIL_BUFFER_BIT

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
  void C_TextureParameterOpenGL(unsigned int aTarget, int aParam, unsigned int aVal);
  //Generate mip levels from GPU texture
  void C_GenMipmapOpenGL(unsigned int aTarget);
  //Activate GPU texture
  void C_ActiveTextureOpenGL(unsigned int aTexture);
  //Deactivate GPU texture
  void C_DeactiveTextureOpenGL(unsigned int aTexture);
  //Deactivate GPU cube map texture
  void C_DeactiveCubemapOpenGL(unsigned int aCubemap);
  //Open vertex attribute stream
  void C_OpenStreamOpenGL(unsigned int aStream);
  //Close vertex attribute stream
  void C_CloseStreamOpenGL(unsigned int aStream);
  //Enable writing into the depth buffer
  void C_EnableDepthMaskOpenGL();
  //Disable writing into the depth buffer
  void C_DisableDepthMaskOpenGL();
  //Set OpenGL buffer as vertex attribute
  void C_VertexAttribPointerOpenGL(unsigned int aIndex, unsigned int aSize,
    unsigned int aType, unsigned int aNormalized, size_t aStride, const void* aData);
  //Render primitive from array data
  void C_DrawArraysOpenGL(unsigned int aMode, int aFirst, size_t aCount);
  //Change pixel arithmetic
  void C_BlendFuncOpenGL(unsigned int aSFactor, unsigned int aDFactor);
  //Enable depth testing
  void C_EnableDepthTestOpenGL();
  //Disable depth testing
  void C_DisableDepthTestOpenGL();
  //Enable texturing
  void C_EnableTextureOpenGL();
  //Disable texturing
  void C_DisableTextureOpenGL();
  //Enable blending
  void C_EnableBlendOpenGL();
  //Disable blending
  void C_DisableBlendOpenGL();
  //Enalbe culling
  void C_EnableCullFaceOpenGL();
  //Disable culling
  void C_DisableCullFaceOpenGL();
  //Enable cubemapping
  void C_EnableCubemapOpenGL();
  //Disable cubemapping
  void C_DisableCubemapOpenGL();
  //Enable multisampling
  void C_EnableMultisampleOpenGL();
  //Disable multisampling
  void C_DisableMultisampleOpenGL();
  //Enable alpha testing
  void C_EnableAlphaTestOpenGL();
  //Disable alpha testing
  void C_DisableAlphaTestOpenGL();
  //Change depth buffer algorithm
  void C_DepthFuncOpenGL(unsigned int aFunc);
  //Change face culling
  void C_CullFaceOpenGL(unsigned int aMode);
  //Get OpenGL vendor
  std::string C_GetVendorOpenGL();
  //Get OpenGL renderer
  std::string C_GetRendererOpenGL();
  //Get OpenGL version
  std::string C_GetVersionOpenGL();
  //Get OpenGL GLSL version
  std::string C_GetGLSLVersionOpenGL();
  //Clear OpenGL buffer
  void C_ClearOpenGL(unsigned int aMask);
  //Set OpenGL clear color
  void C_ClearColorOpenGL(float aR, float aG, float aB, float aA);
  //Set OpenGL viewport
  void C_ViewportOpenGL(int aX, int aY, size_t aW, size_t aH);
  //Draw screen quad
  void C_DrawScreenQuadOpenGL();

}
