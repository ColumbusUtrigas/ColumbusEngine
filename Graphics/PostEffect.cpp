/************************************************
*                 PostEffect.cpp                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.10.2017                  *
*************************************************/

#include <Graphics/PostEffect.h>

namespace C
{

  //////////////////////////////////////////////////////////////////////////////
  C_PostEffect::C_PostEffect()
  {
    mFB = new C_Framebuffer();
    mTB = new C_Texture(NULL, 640, 480, true);
    mDepth = new C_Texture();
    mDepth->loadDepth(NULL, 640, 480, true);
    mRB = new C_Renderbuffer();

    mFB->setTexture2D(C_FRAMEBUFFER_COLOR_ATTACH, mTB->getID());
    mFB->setTexture2D(C_FRAMEBUFFER_DEPTH_ATTACH, mDepth->getID());
    //mRB->storage(C_RENDERBUFFER_DEPTH_24, 640, 480);
    //mFB->setRenderbuffer(C_FRAMEBUFFER_DEPTH_ATTACH, mRB->getID());
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_PostEffect::setShader(C_Shader* aShader)
  {
    mShader = aShader;
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_PostEffect::addAttrib(C_PostEffectAttributeInt aAttrib)
  {
    mAttribsInt.push_back(aAttrib);
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_PostEffect::addAttrib(C_PostEffectAttributeFloat aAttrib)
  {
    mAttribsFloat.push_back(aAttrib);
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_PostEffect::addAttrib(C_PostEffectAttributeVector2 aAttrib)
  {
    mAttribsVector2.push_back(aAttrib);
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_PostEffect::addAttrib(C_PostEffectAttributeVector3 aAttrib)
  {
    mAttribsVector3.push_back(aAttrib);
  }
  void C_PostEffect::addAttrib(C_PostEffectAttributeVector4 aAttrib)
  {
    mAttribsVector4.push_back(aAttrib);
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_PostEffect::clearAttribs()
  {
    mAttribsInt.clear();
    mAttribsFloat.clear();
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_PostEffect::bind(C_Vector4 aClear, C_Vector2 aWindowSize)
  {
    mTB->load(NULL, aWindowSize.x, aWindowSize.y, true);
    mDepth->loadDepth(NULL, aWindowSize.x, aWindowSize.y, true);
    //mRB->storage(C_RENDERBUFFER_DEPTH_24_STENCIL_8, aWindowSize.x, aWindowSize.y);
    mFB->prepare(aClear, aWindowSize);
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_PostEffect::draw()
  {
    if (mShader == nullptr) return;

    mTB->generateMipmap();
    mDepth->generateMipmap();
    mShader->bind();
    
    mShader->setUniform1i("uColor", 0);
    mTB->sampler2D(0);

    mShader->setUniform1i("uDepth", 1);
    mDepth->sampler2D(1);

    for (int i = 0; i < mAttribsInt.size(); i++)
      mShader->setUniform1i(mAttribsInt[i].name.c_str(), mAttribsInt[i].value);

    for (int i = 0; i < mAttribsFloat.size(); i++)
      mShader->setUniform1f(mAttribsFloat[i].name.c_str(), mAttribsFloat[i].value);

    for (int i = 0; i < mAttribsVector2.size(); i++)
      mShader->setUniform2f(mAttribsVector2[i].name.c_str(), mAttribsVector2[i].value);

    for (int i = 0; i < mAttribsVector3.size(); i++)
      mShader->setUniform3f(mAttribsVector3[i].name.c_str(), mAttribsVector3[i].value);

    for (int i = 0; i < mAttribsVector4.size(); i++)
      mShader->setUniform4f(mAttribsVector4[i].name.c_str(), mAttribsVector4[i].value);

    C_DrawScreenQuadOpenGL();

    C_Texture::unbind();
    C_Shader::unbind();
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_PostEffect::unbind()
  {
    C_CloseStreamOpenGL(0);
    C_CloseStreamOpenGL(1);
    C_CloseStreamOpenGL(2);
    C_CloseStreamOpenGL(3);
    C_CloseStreamOpenGL(4);

    C_Cubemap::unbind();
    C_Buffer::unbind();
    C_Texture::unbind();
    C_Shader::unbind();
    C_Framebuffer::unbind();
    C_Renderbuffer::unbind();

    C_DisableDepthTestOpenGL();
    C_DisableBlendOpenGL();
    C_DisableAlphaTestOpenGL();
  }
  //////////////////////////////////////////////////////////////////////////////
  C_PostEffect::~C_PostEffect()
  {

  }

}
