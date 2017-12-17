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
		mNoneShader = new C_Shader("Data/Shaders/post.vert", "Data/Shaders/NonePost.frag");

		mNoneEffect.setShader(mNoneShader);
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
			{
				mesh->setLights(mLights);
				if (mCamera != nullptr)
					mesh->setCamera(*mCamera);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::particlesWorkflow()
	{
		for (auto PS : mMeshes)
		{
			C_ParticleSystem* ps =
				static_cast<C_ParticleSystem*>(PS.second->getComponent("ParticleSystem"));

			if (ps != nullptr)
			{
				ps->setLights(mLights);
				if (mCamera != nullptr)
					ps->setCamera(*mCamera);
			}
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
	void C_Scene::setCamera(const C_Camera* aCamera)
	{
		mCamera = const_cast<C_Camera*>(aCamera);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::setContextSize(const C_Vector2 aContextSize)
	{
		mContextSize = static_cast<C_Vector2>(aContextSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::update()
	{
		lightWorkflow();
		meshWorkflow();
		particlesWorkflow();

		for (auto Mesh : mMeshes)
			Mesh.second->update();
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::render()
	{
		C_EnableDepthTestOpenGL();
		C_EnableBlendOpenGL();
		C_EnableAlphaTestOpenGL();

		mNoneEffect.bind(C_Vector4(1, 1, 1, 0), mContextSize);
		
		if (mSkybox != nullptr)
			mSkybox->draw();

		for (auto Object : mMeshes)
			C_Render::render(Object.second);

		mNoneEffect.unbind();

		mNoneEffect.draw();
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Scene::~C_Scene()
	{

	}

}




