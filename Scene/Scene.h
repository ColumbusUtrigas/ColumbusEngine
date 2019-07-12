#pragma once

#include <Audio/AudioSystem.h>
#include <Graphics/Skybox.h>
#include <Graphics/Camera.h>
#include <Physics/PhysicsWorld.h>
#include <Scene/GameObject.h>
#include <Scene/GameObjectsHolder.h>
#include <System/Timer.h>
#include <Core/SmartPointer.h>
#include <Core/Types.h>

#include <Graphics/Texture.h>
#include <Graphics/Shader.h>
#include <Graphics/Mesh.h>

#include <Resources/ResourceManager.h>

namespace Columbus
{

	class Scene
	{
	private:
		friend class Renderer;
		friend class Editor;
		friend class EditorPanelInspector;
		friend class ResourcesViewerTexture;
		friend class ResourcesViewerShader;
		friend class ResourcesViewerMaterial;
		friend class ResourcesViewerMesh;

		ResourceManager<Texture> TexturesManager;
		ResourceManager<ShaderProgram> ShadersManager;
		ResourceManager<Material> MaterialsManager;
		ResourceManager<Mesh> MeshesManager;
		ResourceManager<Sound> SoundsManager;

		void SerializeTexturesManager(JSON& J);
		void SerializeShadersManager(JSON& J);
		void SerializeMaterialsManager(JSON& J);
		void SerializeMeshesManager(JSON& J);
		void SerializeSoundsManager(JSON& J);
		void SerializeObjects(JSON& J);

		void DeserializeTexturesManager(JSON& J);
		void DeserializeShadersManager(JSON& J);
		void DeserializeMaterialsManager(JSON& J);
		void DeserializeMeshesManager(JSON& J);
		void DeserializeSoundsManager(JSON& J);
		void DeserializeObjects(JSON& J);

		Vector<AudioSource*> AudioSources;
		std::vector<Light*> Lights;

		Timer DeltaTime;
		PhysicsWorld PhysWorld;

		void RigidbodyWorkflow();
		void RigidbodyPostWorkflow();
	public:
		AudioSystem Audio;
		float TimeFactor = 1.0f;

		String SkyPath;
		Skybox* Sky = nullptr;
		Camera* MainCamera = nullptr;
		AudioListener* Listener = nullptr;

		GameObjectsHolder Objects;
	public:
		Scene();

		bool Load(const char* FileName);
		bool Save(const char* FileName);

		void Add(GameObject&& InObject)
		{
			GameObject* New = new GameObject(std::move(InObject));
			Objects.Add(SmartPointer<GameObject>(New), New->Name);
		}

		void AddEmpty()
		{
			GameObject GO;
			String Name = "Object ";
			for (uint32 i = 0;; i++)
			{
				if (Objects.Find(Name + std::to_string(i).c_str()) == nullptr)
				{
					Name += std::to_string(i).c_str();
					break;
				}
			}
			GO.Name = Name;
			Add(std::move(GO));
		}

		void SetSkybox(Skybox* InSky) { Sky = InSky; }
		void SetCamera(Camera& InMainCamera) { MainCamera = &InMainCamera; }
		void SetAudioListener(AudioListener& InListener) { Listener = &InListener; }

		void Update();
		void Render();

		~Scene();
	};

}


