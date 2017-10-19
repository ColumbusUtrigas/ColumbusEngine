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
		mNegativePost = new C_Shader("Data/Shaders/post.vert", "Data/Shaders/NegativePost.frag");
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
		prepareScene();

		TB->load(NULL, mWindowSize.x, mWindowSize.y, true);
		RB->storage(C_RENDERBUFFER_DEPTH_24_STENCIL_8, mWindowSize.x, mWindowSize.y);
		FB->prepare(C_Vector4(1, 1, 1, 0), mWindowSize);

		renderScene();

		C_Framebuffer::unbind();
		
		unbindAll();

		TB->generateMipmap();

		mPostProcess->bind();
		mPostProcess->setUniform2f("uWindowSize", mWindowSize);

		TB->bind();
		C_DrawScreenQuadOpenGL();

		C_Texture::unbind();
		C_Shader::unbind();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Unbind all OpenGL varyables
	void C_Render::unbindAll()
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
	//Enable all OpenGL varyables
	void C_Render::enableAll()
	{
		C_OpenStreamOpenGL(0);
		C_OpenStreamOpenGL(1);
		C_OpenStreamOpenGL(2);
		C_OpenStreamOpenGL(3);
		C_OpenStreamOpenGL(4);

		C_EnableDepthTestOpenGL();
		C_EnableBlendOpenGL();
		C_EnableAlphaTestOpenGL();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Prepare scene to rendering
	void C_Render::prepareScene()
	{
		for (size_t i = 0; i < mMeshes.size(); i++)
		{
			if (mCamera != nullptr)
				if (mMeshes[i] != nullptr)
				{
					mMeshes[i]->setCamera(*mCamera);
					mMeshes[i]->setLights(mLights);
				}
		}

		for (size_t i = 0; i < mParticleEmitters.size(); i++)
		{
			if (mCamera != nullptr)
				if (mParticleEmitters[i] != nullptr)
					mParticleEmitters[i]->setCameraPos(mCamera->pos());
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Render scene
	void C_Render::renderScene()
	{
		if (mSkybox != nullptr)
			mSkybox->draw();

		for (size_t i = 0; i < mMeshes.size(); i++)
			if (mMeshes[i] != nullptr)
				mMeshes[i]->draw();

		for (size_t i = 0; i < mParticleEmitters.size(); i++)
			if (mParticleEmitters[i] != nullptr)
				mParticleEmitters[i]->draw();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Render::~C_Render()
	{

	}

}
