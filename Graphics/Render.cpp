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
		FB = new C_Framebuffer();
		TB = new C_Texture(NULL, 640, 480, true);
		RB = new C_Renderbuffer();

		FB->setTexture2D(C_FRAMEBUFFER_COLOR_ATTACH, TB->getID());
		RB->storage(C_RENDERBUFFER_DEPTH_24, 640, 480);
		FB->setRenderbuffer(C_FRAMEBUFFER_DEPTH_ATTACH, RB->getID());

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
	//Add light
	void C_Render::add(C_Light* aLight)
	{
		mLights.push_back(aLight);
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
		enableAll();

		for (size_t i = 0; i < mMeshes.size(); i++)
		{
			if (mCamera != nullptr)
				if (mMeshes[i] != nullptr)
					mMeshes[i]->setCamera(*mCamera);
		}

		for (size_t i = 0; i < mParticleEmitters.size(); i++)
		{
			if (mCamera != nullptr)
				if (mParticleEmitters[i] != nullptr)
					mParticleEmitters[i]->setCameraPos(mCamera->pos());
		}

		TB->load(NULL, mWindowSize.x, mWindowSize.y, true);
		RB->storage(C_RENDERBUFFER_DEPTH_24_STENCIL_8, mWindowSize.x, mWindowSize.y);

		FB->bind();
		glViewport(0, 0, mWindowSize.x, mWindowSize.y);

		glClearColor(1, 1, 1, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		//FB->prepare(C_Vector4(1, 1, 1, 0), mWindowSize);

		if (mSkybox != nullptr)
			mSkybox->draw();

		for (size_t i = 0; i < mMeshes.size(); i++)
			if (mMeshes[i] != nullptr)
				mMeshes[i]->draw();

		for (size_t i = 0; i < mParticleEmitters.size(); i++)
			if (mParticleEmitters[i] != nullptr)
				mParticleEmitters[i]->draw();

		C_Framebuffer::unbind();
		unbindAll();

		TB->generateMipmap();

		mPostProcess->bind();
		mPostProcess->setUniform2f("uWindowSize", mWindowSize);

		TB->bind();
		drawQuad();

		C_Texture::unbind();

		C_Shader::unbind();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Draw screen quad
	void C_Render::drawQuad()
	{
		glDepthMask(GL_FALSE);

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

		glDepthMask(GL_TRUE);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Unbind all OpenGL varyables
	void C_Render::unbindAll()
	{
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		C_Cubemap::unbind();
		C_Buffer::unbind();
		C_Texture::unbind();
		C_Shader::unbind();
		C_Framebuffer::unbind();
		C_Renderbuffer::unbind();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Enable all OpenGL varyables
	void C_Render::enableAll()
	{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Render::~C_Render()
	{

	}

}
