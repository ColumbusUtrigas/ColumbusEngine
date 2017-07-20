/************************************************
*              		 Skybox.cpp                   *
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
	C_Skybox::C_Skybox(C_Cubemap* aCubemap)
	{
		mCubemap = aCubemap;

		std::vector<float> v;

		for (size_t i = 0; i < 108; i++)
		{
			v.push_back(skyboxVertices[i]);
		}

		mBuf = new C_Buffer(v.data(), v.size() * sizeof(float));

		mShader = new C_Shader("Data/Shaders/skybox.vert", "Data/Shaders/skybox.frag");
	}
	//////////////////////////////////////////////////////////////////////////////
	//Draw skybox
	void C_Skybox::draw()
	{
		if (mShader != nullptr && mCubemap != nullptr)
		{
			glDepthMask(GL_FALSE);

			if (mBuf == nullptr)
				return;
			mBuf->bind();
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			mShader->bind();

			glm::mat4 view = glm::mat4(glm::mat3(C_GetViewMatrix()));

			mShader->setUniformMatrix("uView", glm::value_ptr(view));
			mShader->setUniformMatrix("uProjection", glm::value_ptr(C_GetProjectionMatrix()));

			glActiveTexture(GL_TEXTURE0);
			mShader->setUniform1i("uSkybox", 0);
			mCubemap->bind();

			glDrawArrays(GL_TRIANGLES, 0, 36);

			C_Buffer::unbind();
			C_Cubemap::unbind();
			C_Shader::unbind();

			glDepthMask(GL_TRUE);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Skybox::~C_Skybox()
	{

	}

}
