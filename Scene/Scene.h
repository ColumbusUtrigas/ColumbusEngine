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
#include <Scene/ParticleSystem.h>
#include <Graphics/Skybox.h>
#include <Graphics/Camera.h>
#include <Graphics/Render.h>
#include <Graphics/PostEffect.h>
#include <System/ResourceManager.h>

namespace Columbus
{

	class C_Scene
	{
	private:
		std::map<unsigned int, C_GameObject*> mMeshes;
		std::vector<C_Light*> mLights;
		std::map<int, C_Texture*> mTextures;

		C_Skybox* mSkybox = nullptr;
		C_Camera* mCamera = nullptr;

		C_PostEffect mNoneEffect;
		C_Shader* mNoneShader = nullptr;

		C_Vector2 mContextSize = C_Vector2(640, 480);

		void lightWorkflow();
		void meshWorkflow();
		void particlesWorkflow();
	public:
		C_Scene();

		bool load(std::string aFile);

		void add(unsigned int aID, C_GameObject* aMesh);
		void setSkybox(const C_Skybox* aSkybox);
		void setCamera(const C_Camera* aCamera);
		void setContextSize(const C_Vector2 aContextSize);

		void update();
		void render();

		~C_Scene();
	};

}
