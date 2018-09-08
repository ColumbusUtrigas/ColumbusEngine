#include <Scene/Scene.h>
#include <Graphics/Device.h>

namespace Columbus
{

	Scene::Scene() : mSkybox(nullptr)
	{
		NoneShader = gDevice->CreateShaderProgram();
		NoneShader->Load("Data/Shaders/PostProcessing.glsl");
		NoneShader->Compile();

		mNoneEffect.AddAttributeName("uResolution");

		mNoneEffect.SetShader(NoneShader);

		PhysWorld.SetGravity(Vector3(0, -9.81, 0));
	}

	void Scene::audioWorkflow()
	{
		for (auto& Object : mObjects)
		{
			ComponentAudioSource* audio = static_cast<ComponentAudioSource*>(Object.second->GetComponent(Component::Type::AudioSource));

			if (audio != nullptr)
			{
				if (!Audio.HasSource(audio->GetSource()))
				{
					Audio.AddSource(audio->GetSource());
				}
			}
		}
	}

	void Scene::lightWorkflow()
	{
		mLights.clear();
		
		for (auto& Object : mObjects)
		{
			ComponentLight* light =
				static_cast<ComponentLight*>(Object.second->GetComponent(Component::Type::Light));

			if (light != nullptr)
			{
				light->Render(Object.second->transform);
				mLights.push_back(light->GetLight());
			}
		}
	}

	void Scene::meshWorkflow()
	{
		for (auto& Object : mObjects)
		{
			ComponentMeshRenderer* mesh =
				static_cast<ComponentMeshRenderer*>(Object.second->GetComponent(Component::Type::MeshRenderer));

			if (mesh != nullptr)
			{
				mesh->SetLights(mLights);
				if (mSkybox != nullptr) mesh->SetReflection(mSkybox->GetCubemap());
				if (mCamera != nullptr) mesh->SetCamera(*mCamera);
			}
		}
	}

	void Scene::meshInstancedWorkflow()
	{
		for (auto& Object : mObjects)
		{
			ComponentMeshInstancedRenderer* mesh =
				static_cast<ComponentMeshInstancedRenderer*>(Object.second->GetComponent(Component::Type::MeshInstancedRenderer));

			if (mesh != nullptr)
			{
				mesh->SetLights(mLights);
				if (mSkybox != nullptr) mesh->SetReflection(mSkybox->GetCubemap());
				if (mCamera != nullptr) mesh->SetCamera(*mCamera);
			}
		}
	}

	
	void Scene::particlesWorkflow()
	{
		for (auto& Object : mObjects)
		{
			ComponentParticleSystem* ps =
				static_cast<ComponentParticleSystem*>(Object.second->GetComponent(Component::Type::ParticleSystem));

			if (ps != nullptr)
			{
				//ps->SetLights(mLights);

				if (ps->GetEmitter() != nullptr)
				{
					if(ps->GetEmitter()->GetParticleEffect() != nullptr)
					{
						ps->GetEmitter()->GetParticleEffect()->setPos(Object.second->transform.GetPos());
					}
				}

				if (mCamera != nullptr) ps->SetCamera(*mCamera);
			}
		}
	}

	void Scene::rigidbodyWorkflow()
	{
		for (auto& Object : mObjects)
		{
			ComponentRigidbody* rb =
				static_cast<ComponentRigidbody*>(Object.second->GetComponent(Component::Type::Rigidbody));

			if (rb != nullptr)
			{
				//rb->Render(Object.second->transform);
				rb->GetRigidbody()->SetTransform(Object.second->transform);
			}
		}
	}

	void Scene::rigidbodyPostWorkflow()
	{
		for (auto& Object : mObjects)
		{
			ComponentRigidbody* rb =
				static_cast<ComponentRigidbody*>(Object.second->GetComponent(Component::Type::Rigidbody));

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

	static Transform SceneGameObjectLoadTransform(Serializer::SerializerXML* Serializer, std::string Element)
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

	static Material* SceneGameObjectLoadMaterial(Serializer::SerializerXML* Serializer, std::string Element)
	{
		Material* Mat = new Material();

		if (Serializer != nullptr)
		{
			std::string Path;

			if (Serializer->GetSubString({ "GameObjects", Element, "Material" }, Path))
			{
				if (Path != "None")
				{
					Mat->loadFromXML(Path);
				}
			} else return nullptr;
		} else return nullptr;

		return Mat;
	}

	static ComponentMeshRenderer* SceneGameObjectLoadComponentMeshRenderer(Serializer::SerializerXML* Serializer, std::string Element, Material* Mat, std::map <uint32, std::vector<Vertex>>* Meshes)
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
					//Mesh->mMat = *Mat;

					MeshRenderer = new ComponentMeshRenderer(Mesh);
				}
				else if (MeshPath == "Cube")
				{
					Mesh* Mesh = gDevice->CreateMesh();
					Mesh->SetVertices(PrimitiveBox());
					//Mesh->mMat = *Mat;

					MeshRenderer = new ComponentMeshRenderer(Mesh);
				}
				else if (MeshPath == "Sphere")
				{
					Mesh* Mesh = gDevice->CreateMesh();
					Mesh->SetVertices(PrimitiveSphere(1, 50, 50));
					//Mesh->mMat = *Mat;

					MeshRenderer = new ComponentMeshRenderer(Mesh);
				}
				else
				{
					if (atoi(MeshPath.c_str()) >= 0)
					{
						Mesh* Mesh = gDevice->CreateMesh();
						Mesh->SetVertices(Meshes->at(atoi(MeshPath.c_str())));
						//Mesh->mMat = *Mat;

						if (Mesh != nullptr)
						{
							MeshRenderer = new ComponentMeshRenderer(Mesh);
						}
					}
				}
			}
		}

		return MeshRenderer;
	}

	static ComponentMeshInstancedRenderer* SceneGameObjectLoadComponentMeshInstancedRenderer(Serializer::SerializerXML* Serializer, std::string Element, Material* Mat, std::map <uint32, std::vector<Vertex>>* Meshes)
	{
		ComponentMeshInstancedRenderer* MeshInstancedRenderer = nullptr;

		if (Serializer != nullptr && Mat != nullptr && Meshes != nullptr)
		{
			std::string MeshPath;

			if (Serializer->GetSubString({ "GameObjects", Element, "Components", "MeshInstancedRenderer", "Mesh" }, MeshPath))
			{
				MeshInstanced* mesh = gDevice->CreateMeshInstanced();

				if (mesh != nullptr)
				{
					mesh->SetVertices(Meshes->at(atoi(MeshPath.c_str())));
					mesh->Mat = *Mat;
					MeshInstancedRenderer = new ComponentMeshInstancedRenderer(mesh);
				}
			}
		}

		return MeshInstancedRenderer;
	}

	static ComponentParticleSystem* SceneGameObjectLoadComponentParticleSystem(Serializer::SerializerXML* Serializer, std::string Element, Material* Mat)
	{
		ComponentParticleSystem* ParticleSystem = nullptr;

		if (Serializer != nullptr && Mat != nullptr)
		{
			std::string ParticleSystemPath;

			if (Serializer->GetSubString({ "GameObjects", Element, "Components", "ParticleSystem", "Particles" }, ParticleSystemPath))
			{
				if (ParticleSystemPath != "None")
				{
					ParticleSystem = new ComponentParticleSystem(new ParticleEmitter(new ParticleEffect(ParticleSystemPath, Mat)));
				}
			}
		}

		return ParticleSystem;
	}

	static ComponentLight* SceneGameObjectLoadComponentLight(Serializer::SerializerXML* Serializer, std::string Element, Vector3 Position)
	{
		ComponentLight* CLight = nullptr;

		if (Serializer != nullptr)
		{
			std::string LightPath;

			if (Serializer->GetSubString({ "GameObjects", Element, "Components", "LightComponent", "Light" }, LightPath))
			{
				if (LightPath != "None")
				{
					CLight = new ComponentLight(new Light(LightPath, Position));
				}
			}
		}

		return CLight;
	}

	static ComponentRigidbody* SceneGameObjectLoadComponentRigidbody(Serializer::SerializerXML* Serializer, std::string Element, Transform Trans, PhysicsShape* Shape)
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

	static PhysicsShape* SceneGameObjectLoadComponentRigidbodyShape(Serializer::SerializerXML* Serializer, std::string Element, std::map<uint32, std::vector<Vertex>>* Meshes)
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
						Shape = new PhysicsShapeConvexHull(Meshes->at(atoi(rbShapeMesh.c_str())));
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

	static ComponentAudioSource* SceneGameObjectLoadComponentAudioSource(Serializer::SerializerXML* Serializer, std::string Element, std::map<uint32, Sound*>* Sounds)
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
					Source->SetSound(Sounds->at(atoi(AudioSourceProperties.SourceSound.c_str())));
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
	static bool SceneLoadGameObject(GameObject& OutObject, Serializer::SerializerXML* Serializer, std::string Element,
		std::map<uint32, std::vector<Vertex>>* Meshes, std::map<uint32, Texture*>* Textures, std::map<uint32, ShaderProgram*>* Shaders,
		std::map<uint32, Sound*>* Sounds, PhysicsWorld* PhysWorld)
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
				material->SetShader(Shaders->at(shaderID));
			}
			else
			{
				return false;
			}

			if (material->getTextureID() != -1)
			{
				material->DiffuseTexture = Textures->at(material->getTextureID());
			}

			if (material->getNormMapID() != -1)
			{
				material->NormalTexture = Textures->at(material->getNormMapID());
			}

			if (material->GetRoughnessMapID() != -1)
			{
				material->RoughnessTexture = Textures->at(material->GetRoughnessMapID());
			}

			if (material->GetMetallicMapID() != -1)
			{
				material->MetallicTexture = Textures->at(material->GetMetallicMapID());
			}

			if (material->GetOcclusionMapID() != -1)
			{
				material->OcclusionMap = Textures->at(material->GetOcclusionMapID());
			}

			if (material->GetEmissionMapID() != -1)
			{
				material->EmissionMap = Textures->at(material->GetEmissionMapID());
			}

			if (material->GetDetailDiffuseMapID() != -1)
			{
				material->DetailDiffuseMap = Textures->at(material->GetDetailDiffuseMapID());
			}

			if (material->GetDetailNormalMapID() != -1)
			{
				material->DetailNormalMap = Textures->at(material->GetDetailNormalMapID());
			}

			ComponentMeshRenderer* MeshRenderer = SceneGameObjectLoadComponentMeshRenderer(Serializer, Element, material, Meshes);
			ComponentMeshInstancedRenderer* MeshInstancedRenderer = SceneGameObjectLoadComponentMeshInstancedRenderer(Serializer, Element, material, Meshes);
			ComponentParticleSystem* ParticleSystem = SceneGameObjectLoadComponentParticleSystem(Serializer, Element, material);
			ComponentLight* Light = SceneGameObjectLoadComponentLight(Serializer, Element, transform.GetPos());
			ComponentRigidbody* Rigidbody = SceneGameObjectLoadComponentRigidbody(Serializer, Element, transform, SceneGameObjectLoadComponentRigidbodyShape(Serializer, Element, Meshes));
			ComponentAudioSource* AudioSource = SceneGameObjectLoadComponentAudioSource(Serializer, Element, Sounds);

			if (MeshRenderer != nullptr)
			{
				OutObject.AddComponent(MeshRenderer);
			}

			if (MeshInstancedRenderer != nullptr)
			{
				OutObject.AddComponent(MeshInstancedRenderer);
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
			OutObject.SetName(name);
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool Scene::load(std::string aFile)
	{
		if (!gDevice)
		{ Log::error("Can't load Scene: " + aFile + " : Device is missing"); return false; }

		Serializer::SerializerXML serializer;

		if (!serializer.Read(aFile, "Scene"))
		{ Log::error("Can't load Scene: " + aFile); return false; }

		uint32 count = 0;
		uint32 texCount = 0;
		uint32 shadersCount = 0;
		uint32 meshesCount = 0;
		uint32 soundsCount = 0;

		std::string path, path1, elem;

		if (serializer.GetSubString({ "Defaults", "Skybox" }, path))
		{
			Image ReflImage;

			if (ReflImage.Load(path))
			{
				Texture* Refl = gDevice->CreateTexture();
				Refl->CreateCube(Texture::Properties(ReflImage.GetWidth(), ReflImage.GetHeight(), 0, ReflImage.GetFormat()));
				Refl->Load(ReflImage);
				mSkybox = new Skybox(Refl);

				Log::success("Default skybox loaded: " + path);
			}
		}

		if (serializer.GetSubInt({ "Resources", "Textures", "Count" }, (int32&)texCount))
		{
			for (uint32 i = 0; i < texCount; i++)
			{
				elem = std::string("Texture") + std::to_string(i);
				if (serializer.GetSubString({ "Resources", "Textures", elem }, path))
				{
					Image Img;

					if (Img.Load(path))
					{
						auto Tex = gDevice->CreateTexture();
						Tex->Create2D(Texture::Properties(Img.GetWidth(), Img.GetHeight(), 0, Img.GetFormat()));
						Tex->Load(Img);

						Log::success("Texture loaded: " + path);
						mTextures[i] = Tex;
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
					tShader->Load(path);
					ShaderPrograms[i] = tShader;
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
					std::vector<Vertex> Vertices;

					if (ModelIsCMF(path))
					{
						ModelLoadCMF(path, Vertices);
						Meshes.insert(std::pair<uint32, std::vector<Vertex>>(i, Vertices));
						Log::success("Mesh loaded: " + path);
					}
					else
					{
						Log::error("Couldn't load mesh: " + path);
						continue;
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
					Sound* snd = new Sound();

					if (snd->Load(path, streaming))
					{
						Sounds.insert(std::pair<uint32, Sound*>(i, snd));
						Log::success("Sound loaded: " + path);
					}
					else
					{
						delete snd;
						continue;
					}
				}
			}
		}

		if (!serializer.GetSubInt({"GameObjects", "Count"}, (int32&)count))
		{ Log::error("Can't load Scene Count: " + aFile); return false; }

		for (uint32 i = 0; i < count; i++)
		{
			std::string elem = "GameObject" + std::to_string(i);

			GameObject Object;

			if (SceneLoadGameObject(Object, &serializer, elem, &Meshes, &mTextures, &ShaderPrograms, &Sounds, &PhysWorld))
			{
				Add(i, std::move(Object));
			}
		}

		//Deleting temperary data

		Meshes.clear();
		return true;
	}
	
	void Scene::setSkybox(const Skybox* aSkybox)
	{
		mSkybox = const_cast<Skybox*>(aSkybox);
	}
	
	void Scene::setCamera(const Camera* aCamera)
	{
		mCamera = const_cast<Camera*>(aCamera);
	}
	
	void Scene::setContextSize(const Vector2 aContextSize)
	{
		mContextSize = static_cast<Vector2>(aContextSize);
	}
	
	GameObject* Scene::getGameObject(const unsigned int aID) const
	{
		return mObjects.at(aID).Get();
	}
	
	GameObject* Scene::getGameObject(const std::string aName) const
	{
		for (auto& Object : mObjects)
		{
			if (Object.second != nullptr)
			{
				if (Object.second->GetName() == aName)
				{
					return Object.second.Get();
				}
			}
		}

		return nullptr;
	}
	
	void Scene::update()
	{
		audioWorkflow();
		lightWorkflow();
		meshWorkflow();
		meshInstancedWorkflow();
		particlesWorkflow();
		rigidbodyWorkflow();

		PhysWorld.Step(Math::TruncToFloat(DeltaTime.elapsed()), 10);
		DeltaTime.reset();

		rigidbodyPostWorkflow();

		if (Listener != nullptr)
		{
			Audio.SetListener(*Listener);
		}

		if (mSkybox && mCamera)
		{
			mSkybox->setCamera(*mCamera);
		}

		for (auto& Object : mObjects)
		{
			if (Object.second != nullptr)
			{
				if (mSkybox != nullptr)
				{
					Object.second->GetMaterial().Reflection = mSkybox->GetCubemap();
				}

				Object.second->Update();
			}
		}
	}
	
	void Scene::render()
	{
		C_EnableDepthTestOpenGL();
		C_EnableBlendOpenGL();
		C_EnableAlphaTestOpenGL();

		mNoneEffect.clearAttribs();
		mNoneEffect.addAttrib({ "uResolution", mContextSize });
		mNoneEffect.Bind(Vector4(1, 1, 1, 0), mContextSize);
		
		if (mSkybox != nullptr)
		{
			mSkybox->draw();
		}

		Render.SetMainCamera(*mCamera);
		Render.SetRenderList(&mObjects);
		Render.CompileLists();
		Render.Render(Renderer::Stage::Opaque);
		Render.Render(Renderer::Stage::Transparent);

		/*for (auto& Object : mObjects)
			if (Object.second->HasComponent(Component::Type::MeshRenderer) || Object.second->HasComponent(Component::Type::MeshInstancedRenderer))
				Object.second->Render();

		for (auto& Object : mObjects)
			if (Object.second->HasComponent(Component::Type::ParticleSystem))
				Object.second->Render();*/

		mNoneEffect.Unbind();
		mNoneEffect.Render();
	}
	
	Scene::~Scene()
	{
		mObjects.clear();
	}

}




