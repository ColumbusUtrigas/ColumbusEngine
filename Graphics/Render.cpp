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
	//Set skybox
	void C_Render::setSkybox(C_Skybox* aSkybox)
	{
		mSkybox = aSkybox;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Render scene
	void C_Render::render()
	{
		if (mSkybox != nullptr)
			mSkybox->draw();

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		for (size_t i = 0; i < mMeshes.size(); i++)
		{
			if (mCamera != nullptr)
				mMeshes[i]->setCamera(*mCamera);

			mMeshes[i]->draw();
		}

		for (size_t i = 0; i < mParticleEmitters.size(); i++)
		{
			if (mCamera != nullptr)
				mParticleEmitters[i]->setCameraPos(mCamera->pos());

			mParticleEmitters[i]->draw();
		}

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		C_Cubemap::unbind();

		C_Buffer::unbind();

		C_Texture::unbind();

		C_Shader::unbind();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Render::~C_Render()
	{

	}

}
