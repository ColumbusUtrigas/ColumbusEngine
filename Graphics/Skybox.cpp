/************************************************
*                  Skybox.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Skybox.h>
#include <Graphics/Device.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	Skybox::Skybox() :
		mBuf(nullptr),
		mCubemap(nullptr),
		mShader(nullptr)
	{
		mBuf = new C_Buffer(skyboxVertices, 108 * sizeof(float), 3);
		mShader = gDevice->createShader("STANDART_SKY_VERTEX", "STANDART_SKY_FRAGMENT");
		mShader->compile();
	}
	//////////////////////////////////////////////////////////////////////////////
	Skybox::Skybox(Cubemap* aCubemap) :
		mBuf(nullptr),
		mCubemap(aCubemap),
		mShader(nullptr)
	{
		mBuf = new C_Buffer(skyboxVertices, 108 * sizeof(float), 3);
		mShader = gDevice->createShader("STANDART_SKY_VERTEX", "STANDART_SKY_FRAGMENT");
		mShader->compile();
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void Skybox::draw()
	{
		if (mShader != nullptr && mBuf != nullptr && mCubemap != nullptr)
		{
			C_DisableDepthMaskOpenGL();

			C_OpenStreamOpenGL(0);
			C_CloseStreamOpenGL(1);
			C_CloseStreamOpenGL(2);
			C_CloseStreamOpenGL(3);
			C_CloseStreamOpenGL(4);

			mBuf->bind();

			C_VertexAttribPointerOpenGL(0, 3, C_OGL_FLOAT, C_OGL_FALSE, 3 * sizeof(float), NULL);

			mShader->bind();

			auto view = mCamera.getViewMatrix();
			view.SetRow(3, Vector4(0, 0, 0, 1));
			view.SetColumn(3, Vector4(0, 0, 0, 1));

			view.Elements(UniformViewMatrix);
			mCamera.getProjectionMatrix().ElementsTransposed(UniformProjectionMatrix);

			mShader->setUniformMatrix("uView", UniformViewMatrix);
			mShader->setUniformMatrix("uProjection", UniformProjectionMatrix);

			C_ActiveTextureOpenGL(C_OGL_TEXTURE0);
			mShader->setUniform1i("uSkybox", 0);
			mCubemap->bind();

			C_DrawArraysOpenGL(C_OGL_TRIANGLES, 0, 36);

			C_Buffer::unbind();

			mShader->unbind();
			mCubemap->unbind();

			C_EnableDepthMaskOpenGL();
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void Skybox::setCamera(const Camera aCamera)
	{
		mCamera = static_cast<Camera>(aCamera);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Skybox::setCubemap(const Cubemap* aCubemap)
	{
		mCubemap = const_cast<Cubemap*>(aCubemap);
	}
	//////////////////////////////////////////////////////////////////////////////
	Cubemap* Skybox::getCubemap() const
	{
		return mCubemap;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Skybox::~Skybox()
	{
		delete mShader;
		delete mBuf;
	}

}
