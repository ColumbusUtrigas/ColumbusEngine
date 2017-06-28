#pragma once

#include <Model.h>
#include <Camera.h>

namespace C
{

	class C_Render
	{
	private:
		std::vector<C_Mesh*> mMeshes;
		C_Camera* mCamera = nullptr;
	public:
		C_Render();

		void add(C_Mesh* aMesh);

		void setMainCamera(C_Camera* aCamera);

		void render();

		~C_Render();
	};

}









