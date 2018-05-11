#include <Scene/Scene.h>
#include <Graphics/Device.h>

namespace Columbus
{

	Scene::Scene() :
		mSkybox(nullptr)
	{
		NoneShader = gDevice->CreateShaderProgram();
		ShaderStage* vert = gDevice->CreateShaderStage();
		ShaderStage* frag = gDevice->CreateShaderStage();
		vert->Load("Data/Shaders/post.vert", ShaderType::Vertex);
		frag->Load("Data/Shaders/NonePost.frag", ShaderType::Fragment);
		NoneShader->AddStage(vert);
		NoneShader->AddStage(frag);
		NoneShader->Compile();

		mNoneEffect.AddAttributeName("uResolution");

		mNoneEffect.SetShader(NoneShader);

		PhysWorld.SetGravity(Vector3(0, -9.81, 0));
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void Scene::lightWorkflow()
	{
		mLights.clear();
		
		for (auto Object : mObjects)
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
	//////////////////////////////////////////////////////////////////////////////
	void Scene::meshWorkflow()
	{
		for (auto Object : mObjects)
		{
			ComponentMeshRenderer* mesh =
				static_cast<ComponentMeshRenderer*>(Object.second->GetComponent(Component::Type::MeshRenderer));

			if (mesh != nullptr)
			{
				mesh->SetLights(mLights);
				if (mSkybox != nullptr) mesh->SetReflection(mSkybox->getCubemap());
				if (mCamera != nullptr) mesh->SetCamera(*mCamera);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void Scene::particlesWorkflow()
	{
		for (auto Object : mObjects)
		{
			ComponentParticleSystem* ps =
				static_cast<ComponentParticleSystem*>(Object.second->GetComponent(Component::Type::ParticleSystem));

			if (ps != nullptr)
			{
				ps->SetLights(mLights);
				if (ps->GetEmitter() != nullptr)
					if(ps->GetEmitter()->getParticleEffect() != nullptr)
						ps->GetEmitter()->getParticleEffect()->setPos(Object.second->transform.GetPos());

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
				rb->Render(Object.second->transform);
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

	static ComponentMeshRenderer* SceneGameObjectLoadComponentMeshRenderer(Serializer::SerializerXML* Serializer, std::string Element, Material* Mat, std::map<uint32, Mesh*>* Meshes)
	{
		ComponentMeshRenderer* MeshRenderer = nullptr;

		if (Serializer != nullptr && Mat != nullptr && Meshes != nullptr)
		{
			std::string MeshPath;

			if (Serializer->GetSubString({ "GameObjects", Element, "Components", "MeshRenderer", "Mesh" }, MeshPath))
			{
				if (MeshPath == "Plane")
				{
					MeshRenderer = new ComponentMeshRenderer(gDevice->createMesh(PrimitivePlane(), *Mat));
				}
				else if (MeshPath == "Cube")
				{
					MeshRenderer = new ComponentMeshRenderer(gDevice->createMesh(PrimitiveBox(), *Mat));
				}
				else if (MeshPath == "Sphere")
				{
					MeshRenderer = new ComponentMeshRenderer(gDevice->createMesh(PrimitiveSphere(1, 50, 50), *Mat));
				}
				else
				{
					if (atoi(MeshPath.c_str()) >= 0)
					{
						Mesh* mesh = Meshes->at(atoi(MeshPath.c_str()));

						if (mesh != nullptr)
						{
							mesh->mMat = *Mat;
							MeshRenderer = new ComponentMeshRenderer(mesh);
						}
					}
				}
			}
		}

		return MeshRenderer;
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

	static PhysicsShape* SceneGameObjectLoadComponentRigidbodyShape(Serializer::SerializerXML* Serializer, std::string Element, std::map<uint32, Mesh*>* Meshes)
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
						Shape = new PhysicsShapeConvexHull(Meshes->at(atoi(rbShapeMesh.c_str()))->Vertices);
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
	/*
	*
	* End of additional functions for loading GameObject
	*
	*/

	static GameObject* SceneLoadGameObject(Serializer::SerializerXML* Serializer, std::string Element,
		std::map<uint32, Mesh*>* Meshes, std::map<uint32, Texture*>* Textures, std::map<uint32, ShaderProgram*>* Shaders,
		PhysicsWorld* PhysWorld)
	{
		GameObject* Object = new GameObject();

		if (Serializer != nullptr && Meshes != nullptr && Textures != nullptr && Shaders != nullptr && PhysWorld != nullptr)
		{
			std::string name;
			int shaderID = -1;

			if (!Serializer->GetSubString({ "GameObjects", Element, "Name" }, name))
			{
				delete Object;
				return nullptr;
			}

			Transform transform = SceneGameObjectLoadTransform(Serializer, Element);
			Material* material = SceneGameObjectLoadMaterial(Serializer, Element);
			if (material == nullptr)
			{
				delete Object;
				return nullptr;
			}

			if (Serializer->GetSubInt({ "GameObjects", Element, "Shader" },shaderID))
			{
				material->SetShader(Shaders->at(shaderID));
			}
			else
			{
				delete Object;
				return nullptr;
			}

			if (material->getTextureID() != -1)
				material->setTexture(Textures->at(material->getTextureID()));
			if (material->getSpecMapID() != -1)
				material->setSpecMap(Textures->at(material->getSpecMapID()));
			if (material->getNormMapID() != -1)
				material->setNormMap(Textures->at(material->getNormMapID()));

			ComponentMeshRenderer* MeshRenderer = SceneGameObjectLoadComponentMeshRenderer(Serializer, Element, material, Meshes);
			ComponentParticleSystem* ParticleSystem = SceneGameObjectLoadComponentParticleSystem(Serializer, Element, material);
			ComponentLight* Light = SceneGameObjectLoadComponentLight(Serializer, Element, transform.GetPos());
			ComponentRigidbody* Rigidbody = SceneGameObjectLoadComponentRigidbody(Serializer, Element, transform, SceneGameObjectLoadComponentRigidbodyShape(Serializer, Element, Meshes));

			if (MeshRenderer != nullptr)
			{
				Object->AddComponent(MeshRenderer);
			}

			if (ParticleSystem != nullptr)
			{
				Object->AddComponent(ParticleSystem);
			}

			if (Light != nullptr)
			{
				Object->AddComponent(Light);
			}

			if (Rigidbody != nullptr)
			{
				PhysWorld->AddRigidbody(Rigidbody->GetRigidbody());
				Object->AddComponent(Rigidbody);
			}

			Object->SetTransform(transform);
			Object->SetName(name);
		}

		return Object;
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

		std::string path, path1, elem;

		if (serializer.GetSubInt({ "Resources", "Textures", "Count" }, (int32&)texCount))
		{
			for (uint32 i = 0; i < texCount; i++)
			{
				elem = std::string("Texture") + std::to_string(i);
				if (serializer.GetSubString({ "Resources", "Textures", elem }, path))
				{
					mTextures.insert(std::pair<uint32, Texture*>(i, gDevice->createTexture(path)));
				}
			}
		}

		if (serializer.GetSubInt({ "Resources", "Shaders", "Count" }, (int32&)shadersCount))
		{
			for (uint32 i = 0; i < shadersCount; i++)
			{
				elem = std::string("Shader") + std::to_string(i);
				if (serializer.GetSubString({ "Resources", "Shaders", elem, "Vertex" }, path) &&
				    serializer.GetSubString({ "Resources", "Shaders", elem, "Fragment" }, path1))
				{
					auto tShader = gDevice->CreateShaderProgram();

					auto VertexStage = gDevice->CreateShaderStage();
					auto FragmentStage = gDevice->CreateShaderStage();
					VertexStage->Load(path, ShaderType::Vertex);
					FragmentStage->Load(path1, ShaderType::Fragment);

					if (VertexStage->IsLoaded() &&
					    FragmentStage->IsLoaded())
					{
						tShader->AddStage(VertexStage);
						tShader->AddStage(FragmentStage);

						ShaderPrograms[i] = tShader;
					}
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
					if (ModelIsCMF(path))
					{
						mMeshes.insert(std::pair<uint32, Mesh*>(i, gDevice->createMesh(ModelLoadCMF(path))));
						Log::success("Mesh loaded: " + path);
					}
					else
					{
						Log::error("Can't load mesh: " + path);
					}
				}
			}
		}

		if (!serializer.GetSubInt({"GameObjects", "Count"}, (int32&)count))
		{ Log::error("Can't load Scene Count: " + aFile); return false; }

		for (uint32 i = 0; i < count; i++)
		{
			std::string elem = "GameObject" + std::to_string(i);
			GameObject* Object = SceneLoadGameObject(&serializer, elem, &mMeshes, &mTextures, &ShaderPrograms, &PhysWorld);

			if (Object != nullptr)
			{
				add(i, Object);
			}
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	void Scene::add(unsigned int aID, GameObject* aMesh)
	{
		mObjects.insert(std::pair<unsigned int, GameObject*>(aID, aMesh));
	}
	//////////////////////////////////////////////////////////////////////////////
	void Scene::setSkybox(const Skybox* aSkybox)
	{
		mSkybox = const_cast<Skybox*>(aSkybox);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Scene::setCamera(const Camera* aCamera)
	{
		mCamera = const_cast<Camera*>(aCamera);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Scene::setContextSize(const Vector2 aContextSize)
	{
		mContextSize = static_cast<Vector2>(aContextSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	GameObject* Scene::getGameObject(const unsigned int aID) const
	{
		return mObjects.at(aID);
	}
	//////////////////////////////////////////////////////////////////////////////
	GameObject* Scene::getGameObject(const std::string aName) const
	{
		for (auto Object : mObjects)
			if (Object.second != nullptr)
				if (Object.second->GetName() == aName)
					return Object.second;

		return nullptr;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void Scene::update()
	{
		lightWorkflow();
		meshWorkflow();
		particlesWorkflow();
		//rigidbodyWorkflow();

		PhysWorld.Step(Math::TruncToFloat(DeltaTime.elapsed()), 10);
		DeltaTime.reset();

		rigidbodyPostWorkflow();

		if (mSkybox && mCamera)
		{
			mSkybox->setCamera(*mCamera);
		}

		for (auto Object : mObjects)
			Object.second->Update();
	}
	//////////////////////////////////////////////////////////////////////////////
	void Scene::render()
	{
		C_EnableDepthTestOpenGL();
		C_EnableBlendOpenGL();
		C_EnableAlphaTestOpenGL();

		mNoneEffect.clearAttribs();
		mNoneEffect.addAttrib({ "uResolution", mContextSize });
		mNoneEffect.Bind(Vector4(1, 1, 1, 0), mContextSize);
		
		if (mSkybox != nullptr)
			mSkybox->draw();

		for (auto Object : mObjects)
			if (Object.second->HasComponent(Component::Type::MeshRenderer))
				Object.second->Render();

		for (auto Object : mObjects)
			if (Object.second->HasComponent(Component::Type::ParticleSystem))
				Object.second->Render();

		mNoneEffect.Unbind();

		mNoneEffect.Render();
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Scene::~Scene()
	{

	}

}




