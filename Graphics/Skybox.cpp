/************************************************
*                  Skybox.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Skybox.h>

namespace C
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

			glm::mat4 view = glm::mat4(glm::mat3(C_GetViewMatrix()));

			mShader->setUniformMatrix("uView", glm::value_ptr(view));
			mShader->setUniformMatrix("uProjection", glm::value_ptr(C_GetProjectionMatrix()));

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
	//Destructor
	C_Skybox::~C_Skybox()
	{

	}

}
