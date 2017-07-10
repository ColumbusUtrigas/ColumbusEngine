#pragma once

#include <Model.h>
#include <Camera.h>
#include <Skybox.h>

namespace C
{

	class C_Render
	{
	private:
		std::vector<C_Mesh*> mMeshes;
		C_Camera* mCamera = nullptr;
		C_Skybox* mSkybox = nullptr;
	public:
		C_Render();

		void add(C_Mesh* aMesh);

		void setMainCamera(C_Camera* aCamera);

		void setSkybox(C_Skybox* aSkybox);

		void render();

		~C_Render();
	};

}









