#include <Scene/Scene.h>
#include <Graphics/Device.h>
#include <Common/JSON/JSON.h>
#include <Profiling/Profiling.h>

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

namespace Columbus
{

	Scene::Scene()
	{
		PhysWorld.SetGravity(Vector3(0, -9.81f, 0));

		MaterialsManager.Add(SmartPointer<Material>(new Material()), "Default");
	}

	void Scene::RigidbodyWorkflow()
	{
		for (auto& Object : Objects.Resources)
		{
			if (Object->Enable)
			{
				ComponentRigidbody* rb = (ComponentRigidbody*)Object->GetComponent(Component::Type::Rigidbody);

				if (rb != nullptr)
				{
					rb->GetRigidbody()->SetTransform(Object->transform);
				}
			}
		}
	}

	void Scene::RigidbodyPostWorkflow()
	{
		for (auto& Object : Objects.Resources)
		{
			if (Object->Enable)
			{
				ComponentRigidbody* rb = (ComponentRigidbody*)Object->GetComponent(Component::Type::Rigidbody);

				if (rb != nullptr)
				{
					Object->transform = rb->GetRigidbody()->GetTransform();
				}
			}
		}
	}

	bool Scene::Load(const char* FileName)
	{
		if (!gDevice)
		{ Log::Error("Can't load Scene: %s: Device is missing", FileName); return false; }

		JSON J;
		if (!J.Load(FileName)) { Log::Error("Can't load Scene: %s", FileName); return false; }

		// Load skybox
		{
			SmartPointer<Texture> Tex(gDevice->CreateTexture());
			if (Tex->Load(J["Defaults"]["Skybox"].GetString().c_str()))
			{
				SkyPath = J["Defaults"]["Skybox"].GetString();
				Sky = new Skybox(Tex.Get());
				Log::Success("Skybox loaded: %s", J["Defaults"]["Skybox"].GetString().c_str());
			}
		}

		// Load textures
		for (uint32 i = 0; i < J["Textures"].GetElementsCount(); i++)
		{
			SmartPointer<Texture> Tex(gDevice->CreateTexture());
			if (Tex->Load(J["Textures"][i].GetString().c_str()))
			{
				TexturesManager.Add(std::move(Tex), J["Textures"][i].GetString());
				Log::Success("Texture loaded: %s", J["Textures"][i].GetString().c_str());
			}
		}

		// Load shaders
		for (uint32 i = 0; i < J["Shaders"].GetElementsCount(); i++)
		{
			SmartPointer<ShaderProgram> Shader(gDevice->CreateShaderProgram());
			if (Shader->Load(J["Shaders"][i].GetString().c_str()))
			{
				ShadersManager.Add(std::move(Shader), J["Shaders"][i].GetString());
			}
		}

		// Load materials
		for (uint32 i = 0; i < J["Materials"].GetElementsCount(); i++)
		{
			SmartPointer<Material> Mat(new Material());
			if (Mat->Load(J["Materials"][i].GetString().c_str(), ShadersManager, TexturesManager))
			{
				MaterialsManager.Add(std::move(Mat), J["Materials"][i].GetString());
			}
		}

		// Load meshes
		for (uint32 i = 0; i < J["Meshes"].GetElementsCount(); i++)
		{
			SmartPointer<Mesh> tMesh(gDevice->CreateMesh());
			if (tMesh->Load(J["Meshes"][i].GetString().c_str()))
			{
				MeshesManager.Add(std::move(tMesh), J["Meshes"][i].GetString());
				Log::Success("Mesh loaded: %s", J["Meshes"][i].GetString().c_str());
			}
		}

		// Load sounds
		for (uint32 i = 0; i < J["Sounds"].GetElementsCount(); i++)
		{
			SmartPointer<Sound> Snd(new Sound());
			if (Snd->Load(J["Sounds"][i]["Name"].GetString().c_str(), J["Sounds"][i]["Streaming"].GetBool()))
			{
				SoundsManager.Add(std::move(Snd), J["Sounds"][i]["Name"].GetString());
				Log::Success("Sound loaded: %s", J["Sounds"][i]["Name"].GetString().c_str());
			}
		}

		// Load objects
		for (uint32 i = 0; i < J["Objects"].GetElementsCount(); i++)
		{
			SmartPointer<GameObject> GO(new GameObject());
			GO->Name = J["Objects"][i]["Name"].GetString();
			GO->transform.Position = J["Objects"][i]["Transform"]["Position"].GetVector3<float>();
			GO->transform.Rotation = J["Objects"][i]["Transform"]["Rotation"].GetVector3<float>();
			GO->transform.Scale = J["Objects"][i]["Transform"]["Scale"].GetVector3<float>();

			if (J["Objects"][i]["Material"].IsNull())
			{
				GO->material = nullptr;
			} else
			{
				GO->material = MaterialsManager.Find(J["Objects"][i]["Material"].GetString());
			}

			if (J["Objects"][i].HasChild("AudioSource"))
			{
				auto Clip = SoundsManager.Find(J["Objects"][i]["AudioSource"]["Clip"].GetString());

				if (Clip != nullptr)
				{
					AudioSource* Source = new AudioSource();
					Source->Gain = (float)J["Objects"][i]["AudioSource"]["Gain"].GetFloat();
					Source->Pitch = (float)J["Objects"][i]["AudioSource"]["Pitch"].GetFloat();
					Source->MinDistance = (float)J["Objects"][i]["AudioSource"]["MinDistance"].GetFloat();
					Source->MaxDistance = (float)J["Objects"][i]["AudioSource"]["MaxDistance"].GetFloat();
					Source->Rolloff = (float)J["Objects"][i]["AudioSource"]["Rolloff"].GetFloat();
					Source->Playing = J["Objects"][i]["AudioSource"]["Playing"].GetBool();
					Source->Looping = J["Objects"][i]["AudioSource"]["Looping"].GetBool();

					if (J["Objects"][i]["AudioSource"]["Mode"].GetString() == "2D")
						Source->SoundMode = AudioSource::Mode::Sound2D;
					else if (J["Objects"][i]["AudioSource"]["Mode"].GetString() == "3D")
						Source->SoundMode = AudioSource::Mode::Sound3D;

					Source->SetSound(Clip);
					GO->AddComponent(new ComponentAudioSource(Source));
				} else
				{
					Log::Warning("%s: Cannot load AudioSource", GO->Name.c_str());
				}
			}

			if (J["Objects"][i].HasChild("MeshRenderer"))
			{
				auto M = MeshesManager.Find(J["Objects"][i]["MeshRenderer"].GetString());
				if (M != nullptr)
				{
					GO->AddComponent(new ComponentMeshRenderer(M));
				} else
				{
					Log::Warning("%s: Cannot load MeshRenderer", GO->Name.c_str());
				}
			}

			Objects.Add(std::move(GO), GO->Name);
		}

		return true;
	}

	bool Scene::Save(const char* FileName)
	{
		JSON J;

		uint32 Counter = 0;

		J["Defaults"]["Skybox"] = SkyPath;

		for (const auto& Elem : TexturesManager.Names)
		{		
			J["Textures"][Counter++] = Elem.second;
		}

		Counter = 0;

		for (const auto& Elem : ShadersManager.Names)
		{
			J["Shaders"][Counter++] = Elem.second;
		}

		Counter = 0;

		for (uint32 i = 1; i < MaterialsManager.Names.size(); i++)
		{
			J["Materials"][i - 1] = MaterialsManager.Names[i];
		}

		for (const auto& Elem : MeshesManager.Names)
		{
			J["Meshes"][Counter++] = Elem.second;
		}

		Counter = 0;

		for (const auto& Elem : SoundsManager.Names)
		{
			J["Sounds"][Counter]["Name"] = Elem.second;
			J["Sounds"][Counter]["Streaming"] = SoundsManager[Elem.first]->IsStreaming();
			Counter++;
		}

		Counter = 0;

		std::map<std::string, size_t> SaveObjects;

		for (const auto& Elem : Objects.ResourcesMap)
		{
			SaveObjects[Elem.first] = Elem.second;
		}

		for (const auto& Elem : SaveObjects)
		{
			auto& Obj = Objects.Resources[Elem.second];

			J["Objects"][Counter]["Name"] = Elem.first;
			J["Objects"][Counter]["Static"] = false;
			J["Objects"][Counter]["Transform"]["Position"] = Obj->transform.Position;
			J["Objects"][Counter]["Transform"]["Rotation"] = Obj->transform.Rotation;
			J["Objects"][Counter]["Transform"]["Scale"] = Obj->transform.Scale;
			if (Obj->material == nullptr)
				J["Objects"][Counter]["Material"] = nullptr;
			else
				J["Objects"][Counter]["Material"] = MaterialsManager.Find(Obj->material);

			auto Source = (ComponentAudioSource*)Obj->GetComponent(Component::Type::AudioSource);
			auto MeshRenderer = (ComponentMeshRenderer*)Obj->GetComponent(Component::Type::MeshRenderer);

			if (Source != nullptr)
			{
				auto S = Source->GetSource();
				if (S != nullptr)
				{
					J["Objects"][Counter]["AudioSource"]["Clip"] = SoundsManager.Find(S->GetSound());
					J["Objects"][Counter]["AudioSource"]["Gain"] = S->Gain;
					J["Objects"][Counter]["AudioSource"]["Pitch"] = S->Pitch;
					J["Objects"][Counter]["AudioSource"]["MinDistance"] = S->MinDistance;
					J["Objects"][Counter]["AudioSource"]["MaxDistance"] = S->MaxDistance;
					J["Objects"][Counter]["AudioSource"]["Rolloff"] = S->Rolloff;
					J["Objects"][Counter]["AudioSource"]["Playing"] = S->Playing;
					J["Objects"][Counter]["AudioSource"]["Looping"] = S->Looping;

					if (S->SoundMode == AudioSource::Mode::Sound2D)
						J["Objects"][Counter]["AudioSource"]["Mode"] = "2D";

					if (S->SoundMode == AudioSource::Mode::Sound3D)
						J["Objects"][Counter]["AudioSource"]["Mode"] = "3D";
				}
			}

			if (MeshRenderer != nullptr)
			{
				J["Objects"][Counter]["MeshRenderer"] = MeshesManager.Find(MeshRenderer->GetMesh());
			}

			Counter++;
		}

		if (!J.Save(FileName)) { Log::Error("Can't save scene: %s", FileName); return false; }
		
		return true;
	}
	
	// This function is fucking slow, I am stupid
	void Scene::Update()
	{
		PROFILE_CPU(ProfileModule::Update);

		float Time = (float)DeltaTime.Elapsed() * TimeFactor;
		DeltaTime.Reset();

		RigidbodyWorkflow();

		Audio.Clear();
		Lights.clear();

		for (auto& Object : Objects.Resources)
		{
			if (Object->Enable)
			{
				// TODO: DOD
				Object->Update(Time);

				auto AudioSource = (ComponentAudioSource*)Object->GetComponent(Component::Type::AudioSource);
				auto Light = (ComponentLight*)Object->GetComponent(Component::Type::Light);
				auto PS = (ComponentParticleSystem*)Object->GetComponent(Component::Type::ParticleSystem);

				if (AudioSource != nullptr) if (!Audio.HasSource(AudioSource->Source)) Audio.AddSource(AudioSource->Source);
				if (Light != nullptr)
				{
					Lights.emplace_back(Light->LightSource);

					if (Light->LightSource->Type == Light::Directional ||
					    Light->LightSource->Type == Light::Spot)
					{
						Vector4 BaseDirection(1, 0, 0, 1);
						Vector4 Direction = BaseDirection * Object->transform.Q.ToMatrix();
						Light->LightSource->Dir = Direction.XYZ().Normalize();
					}
				}
				if (PS != nullptr) PS->Emitter.CameraPosition = MainCamera->Pos;
			}
		}

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
	
	Scene::~Scene()
	{
		delete Sky;
	}

}


