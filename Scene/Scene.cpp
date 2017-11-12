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

	C_Scene::C_Scene()
	{

	}

	void C_Scene::add(unsigned int aID, C_GameObject* aMesh)
	{
		mMeshes.insert(std::pair<unsigned int, C_GameObject*>(aID, aMesh));
	}

	void C_Scene::update()
	{
		for (auto Mesh : mMeshes)
			Mesh.second->update();
	}

	void C_Scene::render()
	{
		for (auto Mesh : mMeshes)
			Mesh.second->render();
	}

	C_Scene::~C_Scene()
	{

	}

}



