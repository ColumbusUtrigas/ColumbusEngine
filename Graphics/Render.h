/************************************************
*              		 Render.h                     *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <Graphics/Model.h>
#include <Graphics/Camera.h>
#include <Graphics/Skybox.h>

namespace C
{

	class C_Render
	{
	private:
		std::vector<C_Mesh*> mMeshes;
		C_Camera* mCamera = nullptr;
		C_Skybox* mSkybox = nullptr;
	public:
		//Constructor
		C_Render();
		//Add mesh
		void add(C_Mesh* aMesh);
		//Set main camera
		void setMainCamera(C_Camera* aCamera);
		//Set skybox
		void setSkybox(C_Skybox* aSkybox);
		//Render scene
		void render();
		//Destructor
		~C_Render();
	};

}
