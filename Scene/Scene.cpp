/************************************************
*              	    Scene.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   11.11.2017                  *
*************************************************/

#include <Scene/Scene.h>

namespace Columbus
{

	C_Scene::C_Scene() :
		mSkybox(nullptr)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::lightWorkflow()
	{
		mLights.clear();
		
		for (auto Mesh : mMeshes)
		{
			C_LightComponent* light =
				static_cast<C_LightComponent*>(Mesh.second->getComponent("LightComponent"));

			if (light != nullptr)
				mLights.push_back(light->getLight());
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::meshWorkflow()
	{
		for (auto Mesh : mMeshes)
		{
			C_MeshRenderer* mesh =
				static_cast<C_MeshRenderer*>(Mesh.second->getComponent("MeshRenderer"));

			if (mesh != nullptr)
				mesh->setLights(mLights);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::add(unsigned int aID, C_GameObject* aMesh)
	{
		mMeshes.insert(std::pair<unsigned int, C_GameObject*>(aID, aMesh));
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::setSkybox(const C_Skybox* aSkybox)
	{
		mSkybox = const_cast<C_Skybox*>(aSkybox);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::update()
	{
		lightWorkflow();
		meshWorkflow();

		for (auto Mesh : mMeshes)
			Mesh.second->update();
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::render()
	{
		if (mSkybox != nullptr)
			mSkybox->draw();

		for (auto Mesh : mMeshes)
			Mesh.second->render();
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Scene::~C_Scene()
	{

	}

}




