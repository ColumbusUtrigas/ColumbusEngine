/************************************************
*                 Render.cpp                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Render.h>

namespace C
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Render::C_Render()
	{
		glGenFramebuffers(1, &FBO);
		glGenTextures(1, &TBO);
		glGenRenderbuffers(1, &RBO);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);

		glBindTexture(GL_TEXTURE_2D, TBO);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TBO, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 640, 480);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

		mPostProcess = new C_Shader("Data/Shaders/post.vert", "Data/Shaders/post.frag");
	}
	//////////////////////////////////////////////////////////////////////////////
	//Add mesh
	void C_Render::add(C_Mesh* aMesh)
	{
		mMeshes.push_back(aMesh);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Add particle emitter
	void C_Render::add(C_ParticleEmitter* aP)
	{
		mParticleEmitters.push_back(aP);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set main camera
	void C_Render::setMainCamera(C_Camera* aCamera)
	{
		mCamera = aCamera;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set window size
	void C_Render::setWindowSize(C_Vector2 aWindowSize)
	{
		mWindowSize = aWindowSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set skybox
	void C_Render::setSkybox(C_Skybox* aSkybox)
	{
		mSkybox = aSkybox;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Render scene
	void C_Render::render()
	{
		//if (mSkybox != nullptr)
			//mSkybox->draw();

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		for (size_t i = 0; i < mMeshes.size(); i++)
		{
			if (mCamera != nullptr)
				mMeshes[i]->setCamera(*mCamera);

			//mMeshes[i]->draw();
		}

		for (size_t i = 0; i < mParticleEmitters.size(); i++)
		{
			if (mCamera != nullptr)
				mParticleEmitters[i]->setCameraPos(mCamera->pos());

			//mParticleEmitters[i]->draw();
		}

		glBindTexture(GL_TEXTURE_2D, TBO);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWindowSize.x, mWindowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mWindowSize.x, mWindowSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
		glViewport(0, 0, mWindowSize.x, mWindowSize.y);

		glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, mWindowSize.x, mWindowSize.y);

		mSkybox->draw();

		mMeshes[0]->draw();
		mMeshes[1]->draw();

		mParticleEmitters[0]->draw();
		/*glBegin(GL_TRIANGLES);
			glVertex2f(0.0, 0.0);
			glVertex2f(1.0, 1.0);
			glVertex2f(0.0, 1.0);

			glVertex2f(0.0, 0.0);
			glVertex2f(1.0, 0.0);
			glVertex2f(1.0, 1.0);
		glEnd();*/

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBindTexture(GL_TEXTURE_2D, TBO);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		C_Cubemap::unbind();

		C_Buffer::unbind();

		C_Texture::unbind();

		C_Shader::unbind();

		mPostProcess->bind();
		mPostProcess->setUniform2f("uWindowSize", mWindowSize);

		glBindTexture(GL_TEXTURE_2D, TBO);

		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_TRIANGLES);
			glTexCoord2f(0.0, 0.0);
			glVertex2f(-1.0, -1.0);

			glTexCoord2f(1.0, 1.0);
			glVertex2f(1.0, 1.0);

			glTexCoord2f(0.0, 1.0);
			glVertex2f(-1.0, 1.0);

			glTexCoord2f(0.0, 0.0);
			glVertex2f(-1.0, -1.0);

			glTexCoord2f(1.0, 0.0);
			glVertex2f(1.0, -1.0);

			glTexCoord2f(1.0, 1.0);
			glVertex2f(1.0, 1.0);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, 0);

		C_Shader::unbind();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Render::~C_Render()
	{

	}

}
