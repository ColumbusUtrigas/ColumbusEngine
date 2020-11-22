#include <Scene/Scene.h>
#include <Graphics/Device.h>
#include <Common/JSON/JSON.h>
#include <Profiling/Profiling.h>
#include <Graphics/DebugRender.h>

#include <Scene/ComponentAudioSource.h>
#include <Scene/ComponentBillboard.h>
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

namespace Columbus
{

	IMPLEMENT_COMPONENT(ComponentAudioSource);
	IMPLEMENT_COMPONENT(ComponentBillboard);
	IMPLEMENT_COMPONENT(ComponentLight);
	IMPLEMENT_COMPONENT(ComponentMeshRenderer);
	IMPLEMENT_COMPONENT(ComponentParticleSystem);
	IMPLEMENT_COMPONENT(ComponentRigidbody);

	std::function<void(Rigidbody * RB)> ComponentRigidbody::OnAdd = [](Rigidbody* RB) {};

	Scene::Scene()
	{
		PhysWorld.SetGravity(Vector3(0, -9.81f, 0));

		ComponentRigidbody::OnAdd = [&](Rigidbody* RB)
		{
			PhysWorld.AddRigidbody(RB);
		};
	}

	void Scene::RigidbodyWorkflow()
	{
		for (auto& Object : Objects.Resources)
		{
			if (Object->Enable)
			{
				auto rb = Object->GetComponent<ComponentRigidbody>();

				if (rb != nullptr && rb->GetRigidbody() != nullptr)
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
				auto rb = Object->GetComponent<ComponentRigidbody>();

				if (rb != nullptr && rb->GetRigidbody() != nullptr)
				{
					Object->transform = rb->GetRigidbody()->GetTransform();
				}
			}
		}
	}

	void Scene::Clear()
	{
		TexturesManager.Clear();
		ShadersManager.Clear();
		MaterialsManager.Clear();
		MeshesManager.Clear();
		SoundsManager.Clear();
		Objects.Clear();
		Audio.Clear();

		_CurrentScene = "";
		SkyPath = "";
		delete Sky;
		Sky = nullptr;
	}

	bool Scene::Load(const std::string& FileName)
	{
		if (!gDevice)
		{ Log::Error("Can't load Scene: %s: Device is missing", FileName.c_str()); return false; }

		JSON J;
		if (!J.Load(FileName.c_str())) { Log::Error("Can't load Scene: %s", FileName.c_str()); return false; }

		Clear();

		// Load skybox
		{
			SmartPointer<Texture> Tex(gDevice->CreateTexture());
			if (Tex->Load(("Data/" + J["Defaults"]["Skybox"].GetString()).c_str()))
			{
				SkyPath = J["Defaults"]["Skybox"].GetString();
				delete Sky;
				Sky = new Skybox(Tex.Get());
				Log::Success("Skybox loaded: %s", J["Defaults"]["Skybox"].GetString().c_str());
			}
		}

		DeserializeTexturesManager(J["Textures"]);
		DeserializeShadersManager(J["Shaders"]);
		DeserializeMaterialsManager(J["Materials"]);
		DeserializeMeshesManager(J["Meshes"]);
		DeserializeSoundsManager(J["Sounds"]);
		DeserializeObjects(J["Objects"]);

		_CurrentScene = FileName;

		return true;
	}

	bool Scene::Save(const std::string& FileName)
	{
		JSON J;

		J["Defaults"]["Skybox"] = SkyPath;

		SerializeTexturesManager(J["Textures"]);
		SerializeShadersManager(J["Shaders"]);
		SerializeMaterialsManager(J["Materials"]);
		SerializeMeshesManager(J["Meshes"]);
		SerializeSoundsManager(J["Sounds"]);
		SerializeObjects(J["Objects"]);

		auto nameStr = FileName.empty() ? _CurrentScene : FileName;

		if (!J.Save(nameStr.c_str())) { Log::Error("Can't save scene: %s", nameStr.c_str()); return false; }

		_CurrentScene = nameStr;
		return true;
	}
	
	// This function is fucking slow, I am stupid
	void Scene::Update()
	{
		PROFILE_CPU(ProfileModule::Update);

		if (FirstFrame)
			DeltaTime.Reset();
		FirstFrame = false;

		float Time = (float)DeltaTime.Elapsed() * TimeFactor;
		DeltaTime.Reset();

		if (EnablePhysicsSimulation)
		{
			RigidbodyWorkflow();
		}

		Audio.Clear();

		for (auto& Object : Objects.Resources)
		{
			if (Object->Enable)
			{
				// TODO: DOD
				Object->Update(Time);

				auto AudioSource = Object->GetComponent<ComponentAudioSource>();
				auto Light = Object->GetComponent<ComponentLight>();
				auto PS = Object->GetComponent<ComponentParticleSystem>();

				if (AudioSource != nullptr) Audio.AddSource(AudioSource->Source);

				if (Light != nullptr)
				{
					if (Light->GetLight().Type == Light::Directional ||
					    Light->GetLight().Type == Light::Spot)
					{
						Vector4 BaseDirection(1, 0, 0, 1);
						Vector4 Direction = BaseDirection * Object->transform.Rotation.ToMatrix();
						Light->GetLight().Dir = Direction.XYZ().Normalize();
					}
				}
				if (PS != nullptr) PS->Emitter.CameraPosition = MainCamera->Pos;
			}
		}

		if (EnablePhysicsSimulation)
		{
			PhysWorld.Step(Time, 10);
		}
		else
		{
			PhysWorld.ClearForces();
		}

		Audio.SetSpeed(TimeFactor);

		if (EnablePhysicsSimulation)
		{
			RigidbodyPostWorkflow();
		}

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


