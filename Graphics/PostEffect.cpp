/************************************************
*                 PostEffect.cpp                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.10.2017                  *
*************************************************/

#include <Graphics/PostEffect.h>
#include <Graphics/Device.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	PostEffect::PostEffect()
	{
		mFB = gDevice->createFramebuffer();
		mTB = gDevice->createTexture(NULL, 640, 480, true);
		mDepth = gDevice->createTexture();
		mDepth->loadDepth(NULL, 640, 480, true);

		mFB->setTexture2D(E_FRAMEBUFFER_COLOR_ATTACH, mTB);
		mFB->setTexture2D(E_FRAMEBUFFER_DEPTH_ATTACH, mDepth);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void PostEffect::setShader(Shader* aShader)
	{
		mShader = aShader;
	}
	//////////////////////////////////////////////////////////////////////////////
	void PostEffect::addAttrib(PostEffectAttributeInt aAttrib)
	{
		mAttribsInt.push_back(aAttrib);
	}
	//////////////////////////////////////////////////////////////////////////////
	void PostEffect::addAttrib(PostEffectAttributeFloat aAttrib)
	{
		mAttribsFloat.push_back(aAttrib);
	}
	//////////////////////////////////////////////////////////////////////////////
	void PostEffect::addAttrib(PostEffectAttributeVector2 aAttrib)
	{
		mAttribsVector2.push_back(aAttrib);
	}
	//////////////////////////////////////////////////////////////////////////////
	void PostEffect::addAttrib(PostEffectAttributeVector3 aAttrib)
	{
		mAttribsVector3.push_back(aAttrib);
	}
	//////////////////////////////////////////////////////////////////////////////
	void PostEffect::addAttrib(PostEffectAttributeVector4 aAttrib)
	{
		mAttribsVector4.push_back(aAttrib);
	}
	//////////////////////////////////////////////////////////////////////////////
	void PostEffect::clearAttribs()
	{
		mAttribsInt.clear();
		mAttribsFloat.clear();
		mAttribsVector2.clear();
		mAttribsVector3.clear();
		mAttribsVector4.clear();
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void PostEffect::bind(Vector4 aClear, Vector2 aWindowSize)
	{
		mTB->load(NULL, Math::TruncToInt(aWindowSize.X), Math::TruncToInt(aWindowSize.Y), true);
		mDepth->loadDepth(NULL, Math::TruncToInt(aWindowSize.X), Math::TruncToInt(aWindowSize.Y), true);
		mFB->prepare(aClear, aWindowSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	void PostEffect::draw()
	{
		if (mShader == nullptr) return;

		mTB->generateMipmap();
		mDepth->generateMipmap();
		mShader->bind();
    
		mShader->setUniform1i("uColor", 0);
		mTB->sampler2D(0);

		mShader->setUniform1i("uDepth", 1);
		mDepth->sampler2D(1);

		for (auto& Attrib : mAttribsInt)
			mShader->setUniform1i(Attrib.name, Attrib.value);

		for (auto& Attrib : mAttribsFloat)
			mShader->setUniform1f(Attrib.name, Attrib.value);

		for (auto& Attrib : mAttribsVector2)
			mShader->setUniform2f(Attrib.name, Attrib.value);

		for (auto& Attrib : mAttribsVector3)
			mShader->setUniform3f(Attrib.name, Attrib.value);

		for (auto& Attrib : mAttribsVector4)
			mShader->setUniform4f(Attrib.name, Attrib.value);

		C_DrawScreenQuadOpenGL();

		if (mTB) mTB->unbind();
		if (mShader) mShader->unbind();
	}
	//////////////////////////////////////////////////////////////////////////////
	void PostEffect::unbind()
	{
		C_CloseStreamOpenGL(0);
		C_CloseStreamOpenGL(1);
		C_CloseStreamOpenGL(2);
		C_CloseStreamOpenGL(3);
		C_CloseStreamOpenGL(4);

		C_Buffer::unbind();

		if (mTB) mTB->unbind();
		if (mShader) mShader->unbind();
		
		mFB->unbind();
		C_Renderbuffer::unbind();

		C_DisableDepthTestOpenGL();
		C_DisableBlendOpenGL();
		C_DisableAlphaTestOpenGL();
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	PostEffect::~PostEffect()
	{

	}

}
