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
		mNonePost = new C_Shader("Data/Shaders/post.vert", "Data/Shaders/NonePost.frag");
		mNegativePost = new C_Shader("Data/Shaders/post.vert", "Data/Shaders/NegativePost.frag");
		mGaussianPost = new C_Shader("Data/Shaders/post.vert", "Data/Shaders/GaussianBlur.frag");

		mNone.setShader(mNonePost);
		mNegative.setShader(mNegativePost);
		mGaussianBlur.setShader(mGaussianPost);
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
		mGaussianBlur.addAttrib({"uWindowSize", mWindowSize});
		mGaussianBlur.addAttrib({"uBlurSize", C_Vector2(2, 2)});

		enableAll();
		prepareScene();

		mNone.bind(C_Vector4(1, 1, 1, 0), mWindowSize);
		renderScene();
		mNone.unbind();

		mNone.draw();

		/*mGaussianBlur.bind(C_Vector4(1, 1, 1, 0), mWindowSize);
		renderScene();
		mGaussianBlur.unbind();

		mNegative.bind(C_Vector4(1, 1, 1, 0), mWindowSize);
		mGaussianBlur.draw();
		mNegative.unbind();

		mNegative.draw();*/

		mGaussianBlur.clearAttribs();
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
