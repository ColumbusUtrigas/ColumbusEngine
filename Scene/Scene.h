#pragma once

#include <Audio/AudioSystem.h>
#include <Graphics/Skybox.h>
#include <Graphics/Camera.h>
#include <Physics/PhysicsWorld.h>
#include <Scene/GameObject.h>
#include <Scene/GameObjectsHolder.h>
#include <Core/SmartPointer.h>
#include <Core/Timer.h>
#include <Core/Types.h>

#include <Graphics/Texture.h>
#include <Graphics/Shader.h>
#include <Graphics/Mesh.h>

#include <Resources/ResourceManager.h>

namespace Columbus
{

	namespace Editor
	{
		class Editor;
		class PanelInspector;
		class ResourcesViewerTexture;
		class ResourcesViewerShader;
		class ResourcesViewerMaterial;
		class ResourcesViewerMesh;
	}

	class Scene
	{
	private:
		friend class Renderer;
		friend class Editor::Editor;
		friend class Editor::PanelInspector;
		friend class Editor::ResourcesViewerTexture;
		friend class Editor::ResourcesViewerShader;
		friend class Editor::ResourcesViewerMaterial;
		friend class Editor::ResourcesViewerMesh;

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

		Timer DeltaTime;
		PhysicsWorld PhysWorld;

		bool FirstFrame = true;

		void RigidbodyWorkflow();
		void RigidbodyPostWorkflow();
	private:
		std::string _CurrentScene;
	public:
		bool EnablePhysicsSimulation = true;

		AudioSystem Audio;
		float TimeFactor = 1.0f;

		String SkyPath;
		Skybox* Sky = nullptr;
		Camera* MainCamera = nullptr;
		AudioListener* Listener = nullptr;

		GameObjectsHolder Objects;
	public:
		Scene();

		auto GetCurrentSceneFilename() const { return _CurrentScene; }

		void Clear();
		bool Load(const std::string& FileName);
		bool Save(const std::string& FileName = "");

		void Add(GameObject* obj)
		{
			Objects.Add(SmartPointer<GameObject>(obj), obj->Name);
		}

		void Add(GameObject&& InObject)
		{
			GameObject* New = new GameObject(std::move(InObject));
			Objects.Add(SmartPointer<GameObject>(New), New->Name);
		}

		void AddEmpty(const String& Name = "")
		{
			GameObject GO;
			String ResName = Name;
			if (Name.empty())
			{
				ResName = "Object ";
				for (uint32 i = 0;; i++)
				{
					if (Objects.Find(ResName + std::to_string(i)) == nullptr)
					{
						ResName += std::to_string(i);
						break;
					}
				}
			}

			GO.Name = ResName;
			Add(std::move(GO));
		}

		void SetSkybox(Skybox* InSky) { Sky = InSky; }
		void SetCamera(Camera& InMainCamera) { MainCamera = &InMainCamera; }
		void SetAudioListener(AudioListener& InListener) { Listener = &InListener; }

		void Update();

		~Scene();
	};

}
