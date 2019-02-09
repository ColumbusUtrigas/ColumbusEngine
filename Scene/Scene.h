#pragma once

#include <Audio/AudioSystem.h>
#include <Graphics/Skybox.h>
#include <Graphics/Camera.h>
#include <Graphics/Render.h>
#include <Physics/PhysicsWorld.h>
#include <Scene/GameObject.h>
#include <System/Timer.h>
#include <Core/SmartPointer.h>
#include <Core/Types.h>

#include <vector>
#include <map>

namespace Columbus
{

	class Scene
	{
	private:
		std::map<uint32, SmartPointer<GameObject>> Objects;
		std::vector<Light*> Lights;
		std::map<uint32, SmartPointer<Texture>> Textures;
		std::map<uint32, SmartPointer<ShaderProgram>> ShaderPrograms;

		std::map<uint32, SmartPointer<Mesh>> Meshes;
		std::map<uint32, SmartPointer<Sound>> Sounds;

		Renderer MainRender;

		Timer DeltaTime;
		PhysicsWorld PhysWorld;

		Skybox* Sky = nullptr;
		Camera* MainCamera = nullptr;
		AudioListener* Listener = nullptr;

		iVector2 ContextSize = iVector2(640, 480);

		void AudioWorkflow();
		void LightWorkflow();
		void MeshWorkflow();
		void ParticlesWorkflow();
		void RigidbodyWorkflow();
		void RigidbodyPostWorkflow();
	public:
		AudioSystem Audio;
	public:
		Scene();

		bool Load(const char* FileName);

		void Add(uint32 ID, GameObject&& InObject)
		{
			Objects.insert(std::make_pair(ID, SmartPointer<GameObject>(new GameObject(std::move(InObject)))));
		}

		void SetSkybox(Skybox* InSky) { Sky = InSky; }
		void SetCamera(Camera* InMainCamera) { MainCamera = InMainCamera; }
		void SetAudioListener(AudioListener* InListener) { Listener = InListener; }
		void SetContextSize(const iVector2& InContextSize) { ContextSize = InContextSize; }

		GameObject* GetGameObject(uint32 ID) const;
		GameObject* GetGameObject(const std::string& Name) const;

		void Update();
		void Render();

		~Scene();
	};

}


