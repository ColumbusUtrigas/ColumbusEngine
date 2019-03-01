#include <Scene/Scene.h>
#include <Graphics/Device.h>
#include <System/Serializer.h>

#include <Scene/ComponentAudioSource.h>
#include <Scene/ComponentLight.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/ComponentRigidbody.h>

#include <Physics/PhysicsShape.h>
#include <Physics/PhysicsShapeBox.h>
#include <Physics/PhysicsShapeCapsule.h>
#include <Physics/PhysicsShapeCone.h>
#include <Physics/PhysicsShapeConvexHull.h>
#include <Physics/PhysicsShapeCylinder.h>
#include <Physics/PhysicsShapeMultiSphere.h>
#include <Physics/PhysicsShapeSphere.h>

#include <Graphics/Particles/ParticleEmitterLoader.h>

#include <Graphics/Primitives.h>

namespace Columbus
{

	Scene::Scene()
	{
		PhysWorld.SetGravity(Vector3(0, -9.81f, 0));
	}

	void Scene::RigidbodyWorkflow()
	{
		for (auto& Object : Objects)
		{
			ComponentRigidbody* rb = static_cast<ComponentRigidbody*>(Object.second->GetComponent(Component::Type::Rigidbody));

			if (rb != nullptr)
			{
				rb->GetRigidbody()->SetTransform(Object.second->transform);
			}
		}
	}

	void Scene::RigidbodyPostWorkflow()
	{
		for (auto& Object : Objects)
		{
			ComponentRigidbody* rb = static_cast<ComponentRigidbody*>(Object.second->GetComponent(Component::Type::Rigidbody));

			if (rb != nullptr)
			{
				Object.second->SetTransform(rb->GetRigidbody()->GetTransform());
			}
		}
	}

	/*
	*
	* Additional functions for loading GameObject
	*
	*/

	static Transform SceneGameObjectLoadTransform(Serializer::SerializerXML* Serializer, const std::string& Element)
	{
		Transform Trans;

		if (Serializer != nullptr)
		{
			Vector3 Position;
			Vector3 Rotation;
			Vector3 Scale(1, 1, 1);

			if (Serializer->GetSubVector3({ "GameObjects", Element, "Transform", "Position" }, Position, { "X", "Y", "Z" }) &&
			    Serializer->GetSubVector3({ "GameObjects", Element, "Transform", "Rotation" }, Rotation, { "X", "Y", "Z" }) &&
			    Serializer->GetSubVector3({ "GameObjects", Element, "Transform", "Scale" }, Scale, { "X", "Y", "Z" }))
			{
				Trans.SetPos(Position);
				Trans.SetRot(Rotation);
				Trans.SetScale(Scale);
			}
		}

		return Trans;
	}

	static Material* SceneGameObjectLoadMaterial(Serializer::SerializerXML* Serializer, const std::string& Element)
	{
		Material* Mat = new Material();

		if (Serializer != nullptr)
		{
			std::string Path;

			if (Serializer->GetSubString({ "GameObjects", Element, "Material" }, Path))
			{
				if (Path != "None")
				{
					Mat->Load(Path.c_str());
				}
			} else return nullptr;
		} else return nullptr;

		return Mat;
	}

	static ComponentMeshRenderer* SceneGameObjectLoadComponentMeshRenderer(Serializer::SerializerXML* Serializer, const std::string& Element, Material* Mat, std::map <uint32, SmartPointer<Mesh>>* Meshes)
	{
		ComponentMeshRenderer* MeshRenderer = nullptr;

		if (Serializer != nullptr && Mat != nullptr && Meshes != nullptr)
		{
			std::string MeshPath;

			if (Serializer->GetSubString({ "GameObjects", Element, "Components", "MeshRenderer", "Mesh" }, MeshPath))
			{
				if (MeshPath == "Plane")
				{
					Mesh* Mesh = gDevice->CreateMesh();
					Mesh->SetVertices(PrimitivePlane());

					if (Mesh != nullptr)
					{
						MeshRenderer = new ComponentMeshRenderer(Mesh);
					}
				}
				else if (MeshPath == "Cube")
				{
					Mesh* Mesh = gDevice->CreateMesh();
					Mesh->SetVertices(PrimitiveBox());

					if (Mesh != nullptr)
					{
						MeshRenderer = new ComponentMeshRenderer(Mesh);
					}
				}
				else if (MeshPath == "Sphere")
				{
					Mesh* Mesh = gDevice->CreateMesh();
					Mesh->SetVertices(PrimitiveSphere(1, 50, 50));

					if (Mesh != nullptr)
					{
						MeshRenderer = new ComponentMeshRenderer(Mesh);
					}
				}
				else
				{
					if (atoi(MeshPath.c_str()) >= 0)
					{
						//Mesh* Mesh = gDevice->CreateMesh();
						//Mesh->SetVertices(Meshes->at(atoi(MeshPath.c_str())));
						Mesh* tMesh = Meshes->at(atoi(MeshPath.c_str())).Get();

						if (tMesh != nullptr)
						{
							MeshRenderer = new ComponentMeshRenderer(tMesh);
						}
					}
				}
			}
		}

		return MeshRenderer;
	}

	static ComponentParticleSystem* SceneGameObjectLoadComponentParticleSystem(Serializer::SerializerXML* Serializer, const std::string& Element, Material* Mat)
	{
		ComponentParticleSystem* ParticleSystem = nullptr;

		if (Serializer != nullptr && Mat != nullptr)
		{
			std::string ParticleSystemPath;

			if (Serializer->GetSubString({ "GameObjects", Element, "Components", "ParticleSystem", "Particles" }, ParticleSystemPath))
			{
				if (ParticleSystemPath != "None")
				{
					ParticleEmitterCPU Emitter;
					ParticleEmitterLoader::Load(Emitter, ParticleSystemPath.c_str());
					ParticleSystem = new ComponentParticleSystem(std::move(Emitter));
				}
			}
		}

		return ParticleSystem;
	}

	static ComponentLight* SceneGameObjectLoadComponentLight(Serializer::SerializerXML* Serializer, const std::string& Element, Vector3 Position)
	{
		ComponentLight* CLight = nullptr;

		if (Serializer != nullptr)
		{
			std::string LightPath;

			if (Serializer->GetSubString({ "GameObjects", Element, "Components", "LightComponent", "Light" }, LightPath))
			{
				if (LightPath != "None")
				{
					CLight = new ComponentLight(new Light(LightPath.c_str(), Position));
				}
			}
		}

		return CLight;
	}

	static ComponentRigidbody* SceneGameObjectLoadComponentRigidbody(Serializer::SerializerXML* Serializer, const std::string& Element, Transform Trans, PhysicsShape* Shape)
	{
		ComponentRigidbody* CRigidbody = nullptr;

		if (Serializer != nullptr && Shape != nullptr)
		{
			struct
			{
				bool Static = false;
				float Mass = 1.0f;
				float Restitution = 0.0f;
				float Friction = 0.3f;
				float RollingFriction = 0.0f;
				float AngularDamping = 0.3f;
				float AngularTreshold = 0.25f;
				float LinearDamping = 0.3f;
				float LinearTreshold = 0.2f;
			} RigidbodyProperties;

			if (Serializer->GetSubBool({ "GameObjects", Element, "Components", "Rigidbody", "Static" }, RigidbodyProperties.Static) &&
				Serializer->GetSubFloat({ "GameObjects", Element, "Components", "Rigidbody", "Mass" }, RigidbodyProperties.Mass) &&
			    Serializer->GetSubFloat({ "GameObjects", Element, "Components", "Rigidbody", "Restitution" }, RigidbodyProperties.Restitution) &&
			    Serializer->GetSubFloat({ "GameObjects", Element, "Components", "Rigidbody", "Friction" }, RigidbodyProperties.Friction) &&
			    Serializer->GetSubFloat({ "GameObjects", Element, "Components", "Rigidbody", "RollingFriction" }, RigidbodyProperties.RollingFriction) &&
			    Serializer->GetSubFloat({ "GameObjects", Element, "Components", "Rigidbody", "AngularDamping" }, RigidbodyProperties.AngularDamping) &&
			    Serializer->GetSubFloat({ "GameObjects", Element, "Components", "Rigidbody", "AngularTreshold" }, RigidbodyProperties.AngularTreshold) &&
			    Serializer->GetSubFloat({ "GameObjects", Element, "Components", "Rigidbody", "LinearDamping" }, RigidbodyProperties.LinearDamping) &&
			    Serializer->GetSubFloat({ "GameObjects", Element, "Components", "Rigidbody", "LinearTreshold" }, RigidbodyProperties.LinearTreshold))
			{
				Rigidbody* rigidbody = new Rigidbody(Trans, Shape);

				rigidbody->SetMass(RigidbodyProperties.Mass);
				rigidbody->SetRestitution(RigidbodyProperties.Restitution);
				rigidbody->SetFriction(RigidbodyProperties.Friction);
				rigidbody->SetRollingFriction(RigidbodyProperties.RollingFriction);
				rigidbody->SetAngularDamping(RigidbodyProperties.AngularDamping);
				rigidbody->SetAngularTreshold(RigidbodyProperties.AngularTreshold);
				rigidbody->SetLinearDamping(RigidbodyProperties.LinearDamping);
				rigidbody->SetLinearTreshold(RigidbodyProperties.LinearTreshold);

				rigidbody->SetStatic(RigidbodyProperties.Static);

				CRigidbody = new ComponentRigidbody(rigidbody);
			}
		}

		return CRigidbody;
	}

	static PhysicsShape* SceneGameObjectLoadComponentRigidbodyShape(Serializer::SerializerXML* Serializer, const std::string& Element, std::map<uint32, SmartPointer<Mesh>>* Meshes)
	{
		PhysicsShape* Shape = nullptr;

		if (Serializer != nullptr && Meshes != nullptr)
		{

			Vector3 rbShapeSize;
			Vector3 rbShapePosition;
			Vector2 rbShapeRadiusHeight;
			std::string rbShapeMesh = "None";
			float rbShapeRadius = 0.0f;
			Serializer::SerializerXML::Element* rbElement = nullptr;

			if (Serializer->GetSubVector3({ "GameObjects", Element, "Components", "Rigidbody", "ShapeBox" }, rbShapeSize, { "X", "Y", "Z" }))
			{
				delete Shape;
				Shape = new PhysicsShapeBox(rbShapeSize);
			}

			if (Serializer->GetSubVector2({ "GameObjects", Element, "Components", "Rigidbody", "ShapeCapsule" }, rbShapeRadiusHeight, { "Radius", "Height" }))
			{
				delete Shape;
				Shape = new PhysicsShapeCapsule(rbShapeRadiusHeight.X, rbShapeRadiusHeight.Y);
			}

			if (Serializer->GetSubVector2({ "GameObjects", Element, "Components", "Rigidbody", "ShapeCone" }, rbShapeRadiusHeight, { "Radius", "Height" }))
			{
				delete Shape;
				Shape = new PhysicsShapeCone(rbShapeRadiusHeight.X, rbShapeRadiusHeight.Y);
			}

			if (Serializer->GetSubString({ "GameObjects", Element, "Components", "Rigidbody", "ShapeConvexHull" }, rbShapeMesh))
			{
				if (rbShapeMesh != "None")
				{
					if (atoi(rbShapeMesh.c_str()) >= 0)
					{
						delete Shape;
						//Shape = new PhysicsShapeConvexHull(Meshes->at(atoi(rbShapeMesh.c_str())));
					}
				}
			}

			if (Serializer->GetSubVector3({ "GameObjects", Element, "Components", "Rigidbody", "ShapeCylinder" }, rbShapeSize, { "X", "Y", "Z" }))
			{
				delete Shape;
				Shape = new PhysicsShapeCylinder(rbShapeSize);
			}

			if (Serializer->GetSubFloat({ "GameObjects", Element, "Components", "Rigidbody", "ShapeSphere" }, rbShapeRadius))
			{
				delete Shape;
				Shape = new PhysicsShapeSphere(rbShapeRadius);
			}

			if ((rbElement = Serializer->GetSubElement({ "GameObjects", Element, "Components", "Rigidbody", "ShapeMultiSphere", "Sphere" })) != nullptr)
			{
				std::vector<Vector3> rbShapePositions;
				std::vector<float> rbShapeRadiuses;
				uint32 Count = 0;

				do
				{
					Serializer->GetVector3(rbElement, rbShapePosition, { "X", "Y", "Z" });
					Serializer->GetFloat(rbElement, rbShapeRadius);
					rbElement = Serializer->NextElement(rbElement, "Sphere");
					rbShapePositions.push_back(rbShapePosition);
					rbShapeRadiuses.push_back(rbShapeRadius);
					Count++;
				} while (rbElement != nullptr);

				delete Shape;
				Shape = new PhysicsShapeMultiSphere(rbShapePositions.data(), rbShapeRadiuses.data(), Count);
			}
		}

		return Shape;
	}

	static ComponentAudioSource* SceneGameObjectLoadComponentAudioSource(Serializer::SerializerXML* Serializer, const std::string& Element, std::map<uint32, SmartPointer<Sound>>* Sounds)
	{
		ComponentAudioSource* CAudioSource = nullptr;

		if (Serializer != nullptr && Sounds != nullptr)
		{
			struct
			{
				bool Playing;
				double Played;
				std::string Mode;
				std::string SourceSound;
				Vector3 Direction;
				float Gain;
				float Pitch;
				float MinDistance;
				float MaxDistance;
				float RolloffFactor;
				bool Looping;
			} AudioSourceProperties;

			if (Serializer->GetSubBool({ "GameObjects", Element, "Components", "AudioSource", "Playing" }, AudioSourceProperties.Playing) &&
				Serializer->GetSubDouble({ "GameObjects", Element, "Components", "AudioSource", "Played" }, AudioSourceProperties.Played) &&
				Serializer->GetSubString({ "GameObjects", Element, "Components", "AudioSource", "Mode" }, AudioSourceProperties.Mode) &&
				Serializer->GetSubString({ "GameObjects", Element, "Components", "AudioSource", "Sound" }, AudioSourceProperties.SourceSound) &&
				Serializer->GetSubVector3({ "GameObjects", Element, "Components", "AudioSource", "Direction" }, AudioSourceProperties.Direction, {"X", "Y", "Z"}) &&
				Serializer->GetSubFloat({ "GameObjects", Element, "Components", "AudioSource", "Gain" }, AudioSourceProperties.Gain) &&
				Serializer->GetSubFloat({ "GameObjects", Element, "Components", "AudioSource", "Pitch" }, AudioSourceProperties.Pitch) &&
				Serializer->GetSubFloat({ "GameObjects", Element, "Components", "AudioSource", "MinDistance" }, AudioSourceProperties.MinDistance) &&
				Serializer->GetSubFloat({ "GameObjects", Element, "Components", "AudioSource", "MaxDistance" }, AudioSourceProperties.MaxDistance) &&
				Serializer->GetSubFloat({ "GameObjects", Element, "Components", "AudioSource", "RolloffFactor" }, AudioSourceProperties.RolloffFactor) &&
				Serializer->GetSubBool({ "GameObjects", Element, "Components", "AudioSource", "Looping" }, AudioSourceProperties.Looping))
			{

				AudioSource* Source = new AudioSource();

				AudioSourceProperties.Playing ? Source->Play() : Source->Stop();
				Source->SetPlayedTime(AudioSourceProperties.Played);

				if (AudioSourceProperties.Mode == "2D")
				{
					Source->SoundMode = AudioSource::Mode::Sound2D;
				}
				else if (AudioSourceProperties.Mode == "3D")
				{
					Source->SoundMode = AudioSource::Mode::Sound3D;
				}

				Source->Direction = AudioSourceProperties.Direction;
				Source->Looping = AudioSourceProperties.Looping;

				if (atoi(AudioSourceProperties.SourceSound.c_str()) >= 0)
				{
					Source->SetSound(Sounds->at(atoi(AudioSourceProperties.SourceSound.c_str())).Get());
				}

				if (AudioSourceProperties.Gain >= 0.0f)          Source->Gain = AudioSourceProperties.Gain;
				if (AudioSourceProperties.Pitch >= 0.0f)         Source->Pitch = AudioSourceProperties.Pitch;
				if (AudioSourceProperties.MinDistance >= 0.0f)   Source->MinDistance = AudioSourceProperties.MinDistance;
				if (AudioSourceProperties.MaxDistance >= 0.0f)   Source->MaxDistance = AudioSourceProperties.MaxDistance;
				if (AudioSourceProperties.RolloffFactor >= 0.0f) Source->Rolloff = AudioSourceProperties.RolloffFactor;

				CAudioSource = new ComponentAudioSource(Source);
			}
		}

		return CAudioSource;
	}
	/*
	*
	* End of additional functions for loading GameObject
	*
	*/
	static bool SceneLoadGameObject(GameObject& OutObject, Serializer::SerializerXML* Serializer, const std::string& Element,
		std::map<uint32, SmartPointer<Mesh>>* Meshes, std::map<uint32, SmartPointer<Texture>>* Textures, std::map<uint32, SmartPointer<ShaderProgram>>* Shaders,
		std::map<uint32, SmartPointer<Sound>>* Sounds, PhysicsWorld* PhysWorld)
	{
		if (Serializer != nullptr && Meshes != nullptr && Textures != nullptr && Shaders != nullptr && PhysWorld != nullptr)
		{
			std::string name;
			int shaderID = -1;

			if (!Serializer->GetSubString({ "GameObjects", Element, "Name" }, name))
			{
				return false;
			}

			Transform transform = SceneGameObjectLoadTransform(Serializer, Element);
			Material* material = SceneGameObjectLoadMaterial(Serializer, Element);
			if (material == nullptr)
			{
				return false;
			}

			if (Serializer->GetSubInt({ "GameObjects", Element, "Shader" }, shaderID))
			{
				material->SetShader(Shaders->at(shaderID).Get());
			}
			else
			{
				delete material;
				return false;
			}

			if (material->GetAlbedoMapID() != -1) material->AlbedoMap = Textures->at(material->GetAlbedoMapID()).Get();
			if (material->GetNormalMapID() != -1) material->NormalMap = Textures->at(material->GetNormalMapID()).Get();
			if (material->GetRoughnessMapID() != -1) material->RoughnessMap = Textures->at(material->GetRoughnessMapID()).Get();
			if (material->GetMetallicMapID() != -1) material->MetallicMap = Textures->at(material->GetMetallicMapID()).Get();
			if (material->GetOcclusionMapID() != -1) material->OcclusionMap = Textures->at(material->GetOcclusionMapID()).Get();
			if (material->GetEmissionMapID() != -1) material->EmissionMap = Textures->at(material->GetEmissionMapID()).Get();
			if (material->GetDetailAlbedoMapID() != -1) material->DetailAlbedoMap = Textures->at(material->GetDetailAlbedoMapID()).Get();
			if (material->GetDetailNormalMapID() != -1) material->DetailNormalMap = Textures->at(material->GetDetailNormalMapID()).Get();

			ComponentMeshRenderer* MeshRenderer = SceneGameObjectLoadComponentMeshRenderer(Serializer, Element, material, Meshes);
			ComponentParticleSystem* ParticleSystem = SceneGameObjectLoadComponentParticleSystem(Serializer, Element, material);
			ComponentLight* Light = SceneGameObjectLoadComponentLight(Serializer, Element, transform.GetPos());
			ComponentRigidbody* Rigidbody = SceneGameObjectLoadComponentRigidbody(Serializer, Element, transform, SceneGameObjectLoadComponentRigidbodyShape(Serializer, Element, Meshes));
			ComponentAudioSource* AudioSource = SceneGameObjectLoadComponentAudioSource(Serializer, Element, Sounds);

			if (MeshRenderer != nullptr)
			{
				OutObject.AddComponent(MeshRenderer);
			}

			if (ParticleSystem != nullptr)
			{
				OutObject.AddComponent(ParticleSystem);
			}

			if (Light != nullptr)
			{
				OutObject.AddComponent(Light);
			}

			if (Rigidbody != nullptr)
			{
				PhysWorld->AddRigidbody(Rigidbody->GetRigidbody());
				OutObject.AddComponent(Rigidbody);
			}

			if (AudioSource != nullptr)
			{
				OutObject.AddComponent(AudioSource);
			}

			OutObject.SetTransform(transform);
			OutObject.SetMaterial(*material);
			OutObject.Name = name;
		}

		return true;
	}
	
	bool Scene::Load(const char* FileName)
	{
		if (!gDevice)
		{ Log::Error("Can't load Scene: %s: Device is missing", FileName); return false; }

		Serializer::SerializerXML serializer;

		if (!serializer.Read(FileName, "Scene"))
		{ Log::Error("Can't load Scene: %s", FileName); return false; }

		uint32 count = 0;
		uint32 texCount = 0;
		uint32 shadersCount = 0;
		uint32 meshesCount = 0;
		uint32 soundsCount = 0;

		std::string path, path1, elem;

		if (serializer.GetSubString({ "Defaults", "Skybox" }, path))
		{
			Image ReflImage;

			if (ReflImage.Load(path.c_str()))
			{
				Texture* Refl = gDevice->CreateTexture();
				Refl->CreateCube(Texture::Properties(ReflImage.GetWidth(), ReflImage.GetHeight(), 0, ReflImage.GetFormat()));
				Refl->Load(ReflImage);
				Sky = new Skybox(Refl);

				Log::Success("Default skybox loaded: %s", path.c_str());
			}
		}

		if (serializer.GetSubInt({ "Resources", "Textures", "Count" }, (int32&)texCount))
		{
			for (uint32 i = 0; i < texCount; i++)
			{
				elem = std::string("Texture") + std::to_string(i);
				if (serializer.GetSubString({ "Resources", "Textures", elem }, path))
				{
					SmartPointer<Texture> Tex(gDevice->CreateTexture());
					if (Tex->Load(path.c_str()))
					{
						Log::Success("Texture loaded: %s", path.c_str());
						Textures[i] = std::move(Tex);
					}
				}
			}
		}

		if (serializer.GetSubInt({ "Resources", "Shaders", "Count" }, (int32&)shadersCount))
		{
			for (uint32 i = 0; i < shadersCount; i++)
			{
				elem = std::string("Shader") + std::to_string(i);

				if (serializer.GetSubString({ "Resources", "Shaders", elem, "Program" }, path))
				{
					auto tShader = gDevice->CreateShaderProgram();
					tShader->Load(path.c_str());
					ShaderPrograms.insert(std::make_pair(i, SmartPointer<ShaderProgram>(tShader)));
				}
			}
		}

		if (serializer.GetSubInt({ "Resources", "Meshes", "Count" }, (int32&)meshesCount))
		{
			for (uint32 i = 0; i < meshesCount; i++)
			{
				elem = std::string("Mesh") + std::to_string(i);
				if (serializer.GetSubString({ "Resources", "Meshes", elem }, path))
				{
					SmartPointer<Mesh> tMesh(gDevice->CreateMesh());
					if (tMesh->Load(path.c_str()))
					{
						Log::Success("Mesh loaded: %s", path.c_str());
						Meshes[i] = std::move(tMesh);
					}
				}
			}
		}

		if (serializer.GetSubInt({ "Resources", "Sounds", "Count" }, (int32&)soundsCount))
		{
			bool streaming = false;

			for (uint32 i = 0; i < soundsCount; i++)
			{
				elem = std::string("Sound") + std::to_string(i);
				if (serializer.GetSubString({ "Resources", "Sounds", elem, "Path" }, path) &&
				    serializer.GetSubBool({ "Resources", "Sounds", elem, "Streaming" }, streaming))
				{
					SmartPointer<Sound> tSound(new Sound());
					if (tSound->Load(path.c_str(), streaming))
					{
						Log::Success("Sound loaded: %s", path.c_str());
						Sounds[i] = std::move(tSound);
					}
				}
			}
		}

		if (!serializer.GetSubInt({"GameObjects", "Count"}, (int32&)count))
		{ Log::Error("Can't load Scene Count: %s", FileName); return false; }

		for (uint32 i = 0; i < count; i++)
		{
			std::string elem = "GameObject" + std::to_string(i);

			GameObject Object;

			if (SceneLoadGameObject(Object, &serializer, elem, &Meshes, &Textures, &ShaderPrograms, &Sounds, &PhysWorld))
			{
				Add(i, std::move(Object));
			}
		}
		return true;
	}
	
	GameObject* Scene::GetGameObject(uint32 ID) const
	{
		return Objects.at(ID).Get();
	}
	
	GameObject* Scene::GetGameObject(const std::string& Name) const
	{
		for (auto& Object : Objects)
		{
			if (Object.second != nullptr)
			{
				if (Object.second->Name == Name)
				{
					return Object.second.Get();
				}
			}
		}

		return nullptr;
	}

	template <> void Scene::Workflow(const std::unordered_set<ComponentAudioSource*>& Pool)
	{
		for (auto i : Pool) if (!Audio.HasSource(i->GetSource())) Audio.AddSource(i->GetSource());
	}

	template <> void Scene::Workflow(const std::unordered_set<ComponentLight*>& Pool)
	{
		Lights.clear();
		for (auto i : Pool) Lights.emplace_back(i->GetLight());
	}

	template <> void Scene::Workflow(const std::unordered_set<ComponentMeshRenderer*>& Pool)
	{
		for (auto i : Pool) i->SetLights(Lights);
	}

	template <> void Scene::Workflow(const std::unordered_set<ComponentParticleSystem*>& Pool)
	{
		for (auto i : Pool) i->Emitter.CameraPosition = MainCamera->Pos;
	}
	
	void Scene::Update()
	{
		float Time = (float)DeltaTime.Elapsed() * TimeFactor;
		DeltaTime.Reset();

		RigidbodyWorkflow();

		for (auto& Obj : Objects)
		{
			Obj.second->Update(Time);

			AudioSet.insert(Obj.second->GetComponent<ComponentAudioSource>());
			LightSet.insert(Obj.second->GetComponent<ComponentLight>());
			MeshSet.insert(Obj.second->GetComponent<ComponentMeshRenderer>());
			ParticleSet.insert(Obj.second->GetComponent<ComponentParticleSystem>());
		}

		AudioSet.erase(nullptr);
		LightSet.erase(nullptr);
		MeshSet.erase(nullptr);
		ParticleSet.erase(nullptr);

		Workflow(AudioSet);
		Workflow(LightSet);
		Workflow(MeshSet);
		Workflow(ParticleSet);

		PhysWorld.Step(Time, 10);
		Audio.SetSpeed(TimeFactor);

		RigidbodyPostWorkflow();

		if (Listener != nullptr)
		{
			Audio.SetListener(*Listener);
		}

		if (Sky != nullptr && MainCamera != nullptr)
		{
			Sky->SetCamera(*MainCamera);
		}
	}
	
	void Scene::Render()
	{
		MainRender.SetContextSize(ContextSize);
		MainRender.SetMainCamera(*MainCamera);
		MainRender.SetSky(Sky);
		MainRender.SetRenderList(&Objects);
		MainRender.Render();
	}
	
	Scene::~Scene()
	{
		delete Sky;
	}

}


