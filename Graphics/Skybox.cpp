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

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Skybox::C_Skybox(C_Cubemap* aCubemap) :
		mBuf(nullptr),
		mShader(nullptr),
		mCubemap(nullptr)
	{
		mCubemap = aCubemap;

		std::vector<float> v;

		for (size_t i = 0; i < 108; i++)
		{
			v.push_back(skyboxVertices[i]);
		}

		mBuf = new C_Buffer(v.data(), v.size() * sizeof(float), 3);

		mShader = new C_Shader("Data/Shaders/skybox.vert", "Data/Shaders/skybox.frag");
	}
	//////////////////////////////////////////////////////////////////////////////
	//Draw skybox
	void C_Skybox::draw()
	{
		if (mShader != nullptr && mCubemap != nullptr)
		{
			C_DisableDepthMaskOpenGL();

			if (mBuf == nullptr)
				return;

			C_OpenStreamOpenGL(0);
			C_CloseStreamOpenGL(1);
			C_CloseStreamOpenGL(2);
			C_CloseStreamOpenGL(3);
			C_CloseStreamOpenGL(4);

			mBuf->bind();

			C_VertexAttribPointerOpenGL(0, 3, C_OGL_FLOAT, C_OGL_FALSE, 3 * sizeof(float), NULL);

			mShader->bind();

			C_Matrix4 view = C_GetViewMatrix();
			view.setRow(3, C_Vector4(0, 0, 0, 1));
			view.setColumn(3, C_Vector4(0, 0, 0, 1));

			mShader->setUniformMatrix("uView", view.elements());
			mShader->setUniformMatrix("uProjection", C_GetProjectionMatrix().elements());

			C_ActiveTextureOpenGL(C_OGL_TEXTURE0);
			mShader->setUniform1i("uSkybox", 0);
			mCubemap->bind();

			C_DrawArraysOpenGL(C_OGL_TRIANGLES, 0, 36);

			C_Buffer::unbind();
			C_Cubemap::unbind();
			C_Shader::unbind();

			C_EnableDepthMaskOpenGL();
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return Cubemap
	C_Cubemap* C_Skybox::getCubemap() const
	{
		return mCubemap;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Skybox::~C_Skybox()
	{
		delete mShader;
		delete mBuf;
	}

}
