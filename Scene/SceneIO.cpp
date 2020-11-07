#include <Scene/Scene.h>
#include <Scene/ComponentAudioSource.h>
#include <Scene/ComponentLight.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/ComponentRigidbody.h>

#include <Graphics/Device.h>
#include <Common/JSON/JSON.h>
#include <System/Log.h>
#include <Core/ThreadPool.h>

namespace Columbus
{

	void Scene::SerializeTexturesManager(JSON& J)
	{
		for (size_t i = 0; i < TexturesManager.Names.size(); i++)
		{		
			J[i] = TexturesManager.Names[i];
		}
	}

	void Scene::SerializeShadersManager(JSON& J)
	{
		for (size_t i = 0; i < ShadersManager.Names.size(); i++)
		{
			J[i] = ShadersManager.Names[i];
		}
	}

	void Scene::SerializeMaterialsManager(JSON& J)
	{
		for (size_t i = 0; i < MaterialsManager.Names.size(); i++)
		{
			auto name = MaterialsManager.Names[i];
			auto path = "Data/Materials/" + name + ".mat";

			J[i] = name;
			MaterialsManager.Resources[i]->Save(path.c_str(),
				ShadersManager, TexturesManager);
		}
	}

	void Scene::SerializeMeshesManager(JSON& J)
	{
		for (size_t i = 0; i < MeshesManager.Names.size(); i++)
		{
			J[i] = MeshesManager.Names[i];
		}
	}

	void Scene::SerializeSoundsManager(JSON& J)
	{
		for (size_t i = 0; i < SoundsManager.Names.size(); i++)
		{
			auto Snd = SoundsManager.Names.find(i);
			J[i]["Name"] = Snd->second;
			J[i]["Streaming"] = SoundsManager[Snd->first]->IsStreaming();
		}
	}

	void Scene::SerializeObjects(JSON& J)
	{
		std::map<String, size_t> SaveObjects;

		// for sorted objects names
		for (const auto& Elem : Objects.ResourcesMap)
		{
			SaveObjects[Elem.first] = Elem.second;
		}

		std::function<void(GameObject * Obj, JSON & JObj)> save_object;
		save_object = [&](GameObject* Obj, JSON& JObj) {
			JObj["Name"] = Obj->Name;
			JObj["Static"] = false;
			Obj->transform.Serialize(JObj["Transform"]);

			if (!Obj->materials.empty() && Obj->materials[0] != nullptr)
				JObj["Material"] = MaterialsManager.Find(Obj->materials[0]);
			else
				JObj["Material"] = nullptr;

			auto Lig = (ComponentLight*)Obj->GetComponent(Component::Type::Light);
			auto Source = (ComponentAudioSource*)Obj->GetComponent(Component::Type::AudioSource);
			auto MeshRenderer = (ComponentMeshRenderer*)Obj->GetComponent(Component::Type::MeshRenderer);
			auto ParticleSystem = (ComponentParticleSystem*)Obj->GetComponent(Component::Type::ParticleSystem);
			auto Rigidbody = (ComponentRigidbody*)Obj->GetComponent(Component::Type::Rigidbody);

			// save light component
			if (Lig != nullptr)
			{
				Lig->GetLight().Serialize(JObj["Light"]);
			}

			if (Source != nullptr)
			{
				auto S = Source->GetSource();
				if (S != nullptr)
				{
					JObj["AudioSource"]["Clip"] = SoundsManager.Find(S->GetSound());
					JObj["AudioSource"]["Gain"] = S->Gain;
					JObj["AudioSource"]["Pitch"] = S->Pitch;
					JObj["AudioSource"]["MinDistance"] = S->MinDistance;
					JObj["AudioSource"]["MaxDistance"] = S->MaxDistance;
					JObj["AudioSource"]["Rolloff"] = S->Rolloff;
					JObj["AudioSource"]["Playing"] = S->Playing;
					JObj["AudioSource"]["Looping"] = S->Looping;

					if (S->SoundMode == AudioSource::Mode::Sound2D)
						JObj["AudioSource"]["Mode"] = "2D";

					if (S->SoundMode == AudioSource::Mode::Sound3D)
						JObj["AudioSource"]["Mode"] = "3D";
				}
			}

			if (MeshRenderer != nullptr)
			{
				JObj["MeshRenderer"] = MeshesManager.Find(MeshRenderer->GetMesh());
			}

			if (ParticleSystem != nullptr)
			{
				ParticleSystem->GetEmitter().Serialize(JObj["ParticleSystem"]);
			}

			if (Rigidbody != nullptr)
			{
				Rigidbody->GetRigidbody()->Serialize(JObj["Rigidbody"]);
			}

			int counter = 0;
			for (auto& child : Obj->GetChildren())
			{
				save_object(child, JObj["Children"][counter++]);
			}
		};

		size_t Counter = 0;

		for (const auto& Elem : SaveObjects)
		{
			auto& Obj = Objects.Resources[Elem.second];
			auto& JObj = J[Counter];

			if (Obj->GetParent() == nullptr)
			{
				save_object(Obj.Get(), JObj);
				Counter++;
			}
		}
	}

	void Scene::DeserializeTexturesManager(JSON& J)
	{
		using namespace std;
		using job_type = tuple<bool, Image, string, string>;

		thread_pool pool(std::thread::hardware_concurrency());
		vector<future<job_type>> load;
		
		auto job = [](string path, string name) -> job_type {
			Image img;
			bool res = img.Load(path);
			return make_tuple(res, move(img), move(path), move(name));
		};

		for (size_t i = 0; i < J.GetElementsCount(); i++)
		{
			auto name = J[i].GetString();
			auto path = "Data/Textures/" + name;
			load.push_back(pool.submit(job, move(path), move(name)));
		}

		for (auto& thr : load)
		{
			auto pair = move(thr.get());
			auto tex = SmartPointer<Texture>(gDevice->CreateTexture());
			if (get<0>(pair) && tex->CreateAndLoad(get<1>(pair)))
			{
				TexturesManager.Add(move(tex), get<3>(pair).c_str());
				Log::Success("Texture loaded: %s", get<2>(pair).c_str());
			}
		}
	}

	void Scene::DeserializeShadersManager(JSON& J)
	{
		for (size_t i = 0; i < J.GetElementsCount(); i++)
		{
			auto name = J[i].GetString();
			auto path = "Data/Shaders/" + name;

			SmartPointer<ShaderProgram> Shader(gDevice->CreateShaderProgram());
			if (Shader->Load(path.c_str()))
			{
				ShadersManager.Add(std::move(Shader), name);
			}
		}
	}

	void Scene::DeserializeMaterialsManager(JSON& J)
	{
		for (size_t i = 0; i < J.GetElementsCount(); i++)
		{
			auto name = J[i].GetString();
			auto path = "Data/Materials/" + name + ".mat";

			SmartPointer<Material> Mat(new Material());
			if (Mat->Load(path.c_str(), ShadersManager, TexturesManager))
			{
				Mat->Name = name.c_str();
				MaterialsManager.Add(std::move(Mat), name);
			}
		}
	}

	void Scene::DeserializeMeshesManager(JSON& J)
	{
		for (size_t i = 0; i < J.GetElementsCount(); i++)
		{
			auto name = J[i].GetString();
			auto path = "Data/Meshes/" + name;

			SmartPointer<Mesh> tMesh(gDevice->CreateMesh());
			if (tMesh->Load(path.c_str()))
			{
				MeshesManager.Add(std::move(tMesh), name);
				Log::Success("Mesh loaded: %s", name.c_str());
			}
		}
	}

	void Scene::DeserializeSoundsManager(JSON& J)
	{
		for (uint32 i = 0; i < J.GetElementsCount(); i++)
		{
			auto name = J[i]["Name"].GetString();
			auto path = "Data/Sounds/" + name;

			SmartPointer<Sound> Snd(new Sound());
			if (Snd->Load(path.c_str(), J[i]["Streaming"].GetBool()))
			{
				SoundsManager.Add(std::move(Snd), name);
				Log::Success("Sound loaded: %s", name.c_str());
			}
		}
	}

	void Scene::DeserializeObjects(JSON& J)
	{
		std::function<GameObject*(JSON&)> load_object;
		load_object = [&](JSON& JObj) {
			SmartPointer<GameObject> GO(new GameObject());
			GO->Name = JObj["Name"].GetString();
			GO->transform.Deserialize(JObj["Transform"]);

			if (JObj["Material"].IsNull())
				GO->material = nullptr;
			else
				GO->material = MaterialsManager.Find(JObj["Material"].GetString());

			if (JObj.HasChild("AudioSource"))
			{
				auto Clip = SoundsManager.Find(JObj["AudioSource"]["Clip"].GetString());

				if (Clip != nullptr)
				{
					AudioSource* Source = new AudioSource();

					Source->Gain = (float)JObj["AudioSource"]["Gain"].GetFloat();
					Source->Pitch = (float)JObj["AudioSource"]["Pitch"].GetFloat();
					Source->MinDistance = (float)JObj["AudioSource"]["MinDistance"].GetFloat();
					Source->MaxDistance = (float)JObj["AudioSource"]["MaxDistance"].GetFloat();
					Source->Rolloff = (float)JObj["AudioSource"]["Rolloff"].GetFloat();
					Source->Playing = JObj["AudioSource"]["Playing"].GetBool();
					Source->Looping = JObj["AudioSource"]["Looping"].GetBool();

					if (JObj["AudioSource"]["Mode"].GetString() == "2D")
						Source->SoundMode = AudioSource::Mode::Sound2D;
					if (JObj["AudioSource"]["Mode"].GetString() == "3D")
						Source->SoundMode = AudioSource::Mode::Sound3D;

					Source->SetSound(Clip);
					GO->AddComponent(new ComponentAudioSource(std::shared_ptr<AudioSource>(Source)));
				}
				else
				{
					Log::Warning("%s: Cannot load AudioSource", GO->Name.c_str());
				}
			}

			// load light component
			if (JObj.HasChild("Light"))
			{
				Light L;
				L.Deserialize(JObj["Light"]);
				GO->AddComponent<ComponentLight>()->GetLight() = L;
			}

			if (JObj.HasChild("MeshRenderer"))
			{
				auto M = MeshesManager.Find(JObj["MeshRenderer"].GetString());
				if (M != nullptr)
					GO->AddComponent(new ComponentMeshRenderer(M));
				else
					Log::Warning("%s: Cannot load MeshRenderer", GO->Name.c_str());
			}

			if (JObj.HasChild("ParticleSystem"))
			{
				ParticleEmitterCPU Emitter;
				Emitter.Deserialize(JObj["ParticleSystem"]);
				GO->AddComponent(new ComponentParticleSystem(std::move(Emitter)));
			}

			if (JObj.HasChild("Rigidbody"))
			{
				auto RB = new Rigidbody();
				RB->Deserialize(JObj["Rigidbody"]);
				GO->AddComponent(new ComponentRigidbody(RB));
			}

			auto goptr = GO.Get();
			Objects.Add(std::move(GO), GO->Name);

			if (JObj.HasChild("Children") && JObj["Children"].IsArray())
			{
				for (int i = 0; i < JObj["Children"].GetElementsCount(); i++)
				{
					auto child = load_object(JObj["Children"][i]);
					goptr->AddChild(child);
				}
			}

			return goptr;
		};

		for (uint32 i = 0; i < J.GetElementsCount(); i++)
		{
			load_object(J[i]);
		}
	}

}


