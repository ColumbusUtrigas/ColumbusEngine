/************************************************
*              	     Scene.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   12.11.2017                  *
*************************************************/
#pragma once

#include <Scene/GameObject.h>
#include <Scene/LightComponent.h>
#include <Scene/MeshRenderer.h>
#include <Graphics/Skybox.h>
#include <Graphics/Camera.h>

namespace Columbus
{

	class C_Scene
	{
	private:
		std::map<unsigned int, C_GameObject*> mMeshes;
		std::vector<C_Light*> mLights;
		C_Skybox* mSkybox = nullptr;
		C_Camera* mCamera = nullptr;

		void lightWorkflow();
		void meshWorkflow();
	public:
		C_Scene();

		void add(unsigned int aID, C_GameObject* aMesh);
		void setSkybox(const C_Skybox* aSkybox);
		void setCamera(const C_Camera* aCamera);

		void update();
		void render();

		~C_Scene();
	};

}
