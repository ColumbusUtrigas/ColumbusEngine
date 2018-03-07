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
#include <Graphics/Primitives.h>
#include <Graphics/PostEffect.h>
#include <Graphics/OpenGL/MeshOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <System/ResourceManager.h>

namespace Columbus
{

	class C_Scene
	{
	private:
		std::map<unsigned int, C_GameObject*> mObjects;
		std::vector<C_Light*> mLights;
		std::map<int, C_Mesh*> mMeshes;
		std::map<int, C_Texture*> mTextures;
		std::map<int, C_Shader*> mShaders;

		C_Skybox* mSkybox = nullptr;
		C_Camera* mCamera = nullptr;

		C_PostEffect mNoneEffect;
		C_Shader* mNoneShader = nullptr;

		C_Vector2 mContextSize = C_Vector2(640, 480);

		void lightWorkflow();
		void meshWorkflow();
		void particlesWorkflow();

		bool loadGameObject(Serializer::C_SerializerXML* aSerializer,
			std::string aElement, unsigned int aID);
	public:
		C_Scene();

		bool load(std::string aFile);

		void add(unsigned int aID, C_GameObject* aMesh);
		void setSkybox(const C_Skybox* aSkybox);
		void setCamera(const C_Camera* aCamera);
		void setContextSize(const C_Vector2 aContextSize);

		C_GameObject* getGameObject(const unsigned int aID) const;
		C_GameObject* getGameObject(const std::string aName) const;

		void update();
		void render();

		~C_Scene();
	};

}
