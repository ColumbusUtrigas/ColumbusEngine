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

	class Scene
	{
	private:
		std::map<unsigned int, GameObject*> mObjects;
		std::vector<Light*> mLights;
		std::map<int, Mesh*> mMeshes;
		std::map<int, Texture*> mTextures;
		std::map<int, Shader*> mShaders;

		Skybox* mSkybox = nullptr;
		Camera* mCamera = nullptr;

		PostEffect mNoneEffect;
		Shader* mNoneShader = nullptr;

		Vector2 mContextSize = Vector2(640, 480);

		void lightWorkflow();
		void meshWorkflow();
		void particlesWorkflow();

		bool loadGameObject(Serializer::SerializerXML* aSerializer,
			std::string aElement, unsigned int aID);
	public:
		Scene();

		bool load(std::string aFile);

		void add(unsigned int aID, GameObject* aMesh);
		void setSkybox(const Skybox* aSkybox);
		void setCamera(const Camera* aCamera);
		void setContextSize(const Vector2 aContextSize);

		GameObject* getGameObject(const unsigned int aID) const;
		GameObject* getGameObject(const std::string aName) const;

		void update();
		void render();

		~Scene();
	};

}
