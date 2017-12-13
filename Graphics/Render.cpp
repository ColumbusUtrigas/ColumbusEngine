/************************************************
*                 Render.cpp                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Render.h>

namespace Columbus
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
	//Set skybox
	void C_Render::setSkybox(C_Skybox* aSkybox)
	{
		mSkybox = aSkybox;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Render scene
	void C_Render::render()
	{
		/*for (auto Mesh : mMeshes)
			if (Mesh != nullptr)
				Mesh->draw();*/
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Render::render(C_GameObject* aObject)
	{
		if (aObject == nullptr) return;

		aObject->render();
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
	//Prepass scene before rendering
	void C_Render::prepassScene()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//Render scene
	void C_Render::renderScene()
	{
		/*if (mSkybox != nullptr)
			mSkybox->draw();

		for (auto Mesh : mMeshes)
			if (Mesh != nullptr)
				Mesh->draw();

		for (auto ParticleEmitter : mParticleEmitters)
			if (ParticleEmitter != nullptr)
				ParticleEmitter->draw();

		mFrameTimer.reset();*/
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Render::~C_Render()
	{

	}

}
