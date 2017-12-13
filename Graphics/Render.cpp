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

	C_Shader* gMeshWhiteShader = nullptr;
	C_Shader* gParticleWhiteShader = nullptr;

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Render::C_Render()
	{
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
	void C_Render::enableDepthPrepass()
	{
		glColorMask(0, 0, 0, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Render::disableDepthPrepass()
	{
		glColorMask(1, 1, 1, 1);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Render::renderDepthPrepass(C_GameObject* aGameObject)
	{
		if (aGameObject == nullptr) return;
		if (aGameObject->hasComponent("MeshRenderer") == false &&
			aGameObject->hasComponent("ParticleSystem") == false) return;

		if (aGameObject->hasComponent("MeshRenderer"))
			if (gMeshWhiteShader == nullptr)
				gMeshWhiteShader = new C_Shader("Data/Shaders/Standart.vert", "Data/Shaders/White.frag");

		if (aGameObject->hasComponent("ParticleSystem"))
			if (gParticleWhiteShader == nullptr)
				gParticleWhiteShader = new C_Shader("Data/Shaders/particle.vert", "Data/Shaders/White.frag");

		if (aGameObject->hasComponent("MeshRenderer"))
		{
			C_Shader* shaderPtr = static_cast<C_MeshRenderer*>(aGameObject->getComponent("MeshRenderer"))->getShader();
			static_cast<C_MeshRenderer*>(aGameObject->getComponent("MeshRenderer"))->setShader(gMeshWhiteShader);
			aGameObject->render();
			static_cast<C_MeshRenderer*>(aGameObject->getComponent("MeshRenderer"))->setShader(shaderPtr);
			return;
		}

		if (aGameObject->hasComponent("ParticleSystem"))
		{
			C_Shader* shaderPtr = static_cast<C_ParticleSystem*>(aGameObject->getComponent("ParticleSystem"))->getShader();
			static_cast<C_ParticleSystem*>(aGameObject->getComponent("ParticleSystem"))->setShader(gParticleWhiteShader);
			aGameObject->render();
			static_cast<C_ParticleSystem*>(aGameObject->getComponent("ParticleSystem"))->setShader(shaderPtr);
			return;
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Render::render(C_GameObject* aGameObject)
	{
		if (aGameObject == nullptr) return;

		aGameObject->render();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Render::~C_Render()
	{

	}

}
