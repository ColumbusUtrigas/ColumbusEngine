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
		if (mShader != nullptr && mCubemap != nullptr)
		{
			C_DisableDepthMaskOpenGL();

			if (mBuf == nullptr) return;

			C_OpenStreamOpenGL(0);
			C_CloseStreamOpenGL(1);
			C_CloseStreamOpenGL(2);
			C_CloseStreamOpenGL(3);
			C_CloseStreamOpenGL(4);

			mBuf->bind();

			C_VertexAttribPointerOpenGL(0, 3, C_OGL_FLOAT, C_OGL_FALSE, 3 * sizeof(float), NULL);

			mShader->bind();

			Matrix4 view = CameraGetViewMatrix();
			view.setRow(3, Vector4(0, 0, 0, 1));
			view.setColumn(3, Vector4(0, 0, 0, 1));

			mShader->setUniformMatrix("uView", view.elements());
			mShader->setUniformMatrix("uProjection", CameraGetProjectionMatrix().elements());

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
