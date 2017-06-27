#include <Render.h>

namespace C
{

	C_Render::C_Render()
	{

	}

	void C_Render::add(C_Mesh* aMesh)
	{
		mMeshes.push_back(aMesh);
	}

	void C_Render::setMainCamera(C_Camera* aCamera)
	{
		mCamera = aCamera;
	}

	void C_Render::render()
	{
		for (size_t i = 0; i < mMeshes.size(); i++)
		{
			if(mCamera != nullptr)
				mMeshes[i]->setCamera(*mCamera);

			mMeshes[i]->draw();
		}
	}

	C_Render::~C_Render()
	{

	}

}












