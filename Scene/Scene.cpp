/************************************************
*              	    Scene.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   11.11.2017                  *
*************************************************/

#include <Scene/Scene.h>
#include <Graphics/Device.h>

namespace Columbus
{

	Scene::Scene() :
		mSkybox(nullptr)
	{
		mNoneShader = gDevice->createShader("Data/Shaders/post.vert", "Data/Shaders/NonePost.frag");
		mNoneShader->compile();

		mNoneEffect.setShader(mNoneShader);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void Scene::lightWorkflow()
	{
		mLights.clear();
		
		for (auto Object : mObjects)
		{
			LightComponent* light =
				static_cast<LightComponent*>(Object.second->getComponent("LightComponent"));

			if (light != nullptr)
			{
				light->render(Object.second->transform);
				mLights.push_back(light->getLight());
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void Scene::meshWorkflow()
	{
		for (auto Object : mObjects)
		{
			MeshRenderer* mesh =
				static_cast<MeshRenderer*>(Object.second->getComponent("MeshRenderer"));

			if (mesh != nullptr)
			{
				mesh->setLights(mLights);
				if (mSkybox != nullptr) mesh->setReflection(mSkybox->getCubemap());
				if (mCamera != nullptr) mesh->setCamera(*mCamera);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void Scene::particlesWorkflow()
	{
		for (auto Object : mObjects)
		{
			ParticleSystem* ps =
				static_cast<ParticleSystem*>(Object.second->getComponent("ParticleSystem"));

			if (ps != nullptr)
			{
				ps->setLights(mLights);
				if (ps->getEmitter() != nullptr || ps->getEmitter()->getParticleEffect() != nullptr)
					ps->getEmitter()->getParticleEffect()->setPos(Object.second->transform.getPos());
				if (mCamera != nullptr) ps->setCamera(*mCamera);
			}
		}
	}
	bool Scene::loadGameObject(Serializer::SerializerXML* aSerializer,
		std::string aElement, unsigned int aID)
	{
		std::string name;
		std::string materialPath;
		int shaderID = -1;
		std::string meshPath;
		std::string particlePath;
		std::string lightPath;

		GameObject* Object = new GameObject();
		Transform transform;
		Vector3 position;
		Vector3 rotation;
		Vector3 scale;
		Material* material = new Material();
		Mesh* mesh;

		if (!aSerializer->getSubString({ "GameObjects", aElement, "Name" }, &name)) return false;
		aSerializer->getSubVector3({ "GameObjects", aElement, "Transform", "Position" }, &position, { "X", "Y", "Z" });
		aSerializer->getSubVector3({ "GameObjects", aElement, "Transform", "Rotation" }, &rotation, { "X", "Y", "Z" });
		aSerializer->getSubVector3({ "GameObjects", aElement, "Transform", "Scale" }, &scale, { "X", "Y", "Z" });

		if (aSerializer->getSubString({ "GameObjects", aElement, "Material" }, &materialPath))
		{
			if (materialPath != "None")
			{
				material->loadFromXML(materialPath);
			}
		} else return false;

		if (aSerializer->getSubInt({ "GameObjects", aElement, "Shader" }, &shaderID))
		{
			material->setShader(mShaders.at(shaderID));
		}
		else return false;

		if (material->getTextureID() != -1)
			material->setTexture(mTextures.at(material->getTextureID()));
		if (material->getSpecMapID() != -1)
			material->setSpecMap(mTextures.at(material->getSpecMapID()));
		if (material->getNormMapID() != -1)
			material->setNormMap(mTextures.at(material->getNormMapID()));

		if (aSerializer->getSubString({"GameObjects", aElement, "Components", "MeshRenderer", "Mesh"}, &meshPath))
		{
			if (meshPath == "Plane")
			{
				Object->addComponent(new MeshRenderer(gDevice->createMesh(PrimitivePlane(), *material)));
			} else if (meshPath == "Cube")
			{
				Object->addComponent(new MeshRenderer(gDevice->createMesh(PrimitiveBox(), *material)));
			} else if (meshPath == "Sphere")
			{
				Object->addComponent(new MeshRenderer(gDevice->createMesh(PrimitiveSphere(1, 50, 50), *material)));
			} else
			{
				if (atoi(meshPath.c_str()) >= 0)
				{
					mesh = mMeshes.at(atoi(meshPath.c_str()));
					if (mesh != nullptr)
					{
						mesh->mMat = *material;
						Object->addComponent(new MeshRenderer(mesh));
					}
				}
			}
		}

		if (aSerializer->getSubString({"GameObjects", aElement, "Components", "ParticleSystem", "Particles"}, &particlePath))
		{
			Object->addComponent(new ParticleSystem(new ParticleEmitter(new ParticleEffect(particlePath, material))));
		}

		if (aSerializer->getSubString({"GameObjects", aElement, "Components", "LightComponent", "Light"}, &lightPath))
		{
			Object->addComponent(new LightComponent(new Light(lightPath, position)));
		}

		transform.setPos(position);
		transform.setRot(rotation);
		transform.setScale(scale);
		Object->setTransform(transform);
		Object->setName(name);

		add(aID, Object);

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

		if (!serializer.read(aFile, "Scene"))
		{ Log::error("Can't load Scene: " + aFile); return false; }

		int count = 0;
		int texCount = 0;
		int shadersCount = 0;
		int meshesCount = 0;
		unsigned int i;

		std::string path, path1, elem;

		if (serializer.getSubInt({ "Resources", "Textures", "Count" }, &texCount))
		{
			for (i = 0; i < static_cast<unsigned int>(texCount); i++)
			{
				elem = std::string("Texture") + std::to_string(i);
				if (serializer.getSubString({ "Resources", "Textures", elem }, &path))
					mTextures.insert(std::pair<int, Texture*>(i, gDevice->createTexture(path)));
			}
		}

		if (serializer.getSubInt({ "Resources", "Shaders", "Count" }, &shadersCount))
		{
			for (i = 0; i < static_cast<unsigned int>(shadersCount); i++)
			{
				elem = std::string("Shader") + std::to_string(i);
				if (serializer.getSubString({ "Resources", "Shaders", elem, "Vertex" }, &path) &&
					serializer.getSubString({ "Resources", "Shaders", elem, "Fragment" }, &path1))
				{
					mShaders.insert(std::pair<int, Shader*>(i, gDevice->createShader(path, path1)));
				}
			}
		}

		if (serializer.getSubInt({ "Resources", "Meshes", "Count" }, &meshesCount))
		{
			for (i = 0; i < static_cast<unsigned int>(meshesCount); i++)
			{
				elem = std::string("Mesh") + std::to_string(i);
				if (serializer.getSubString({ "Resources", "Meshes", elem }, &path))
				{
					if (ModelIsCMF(path))
					{
						mMeshes.insert(std::pair<int, Mesh*>(i, gDevice->createMesh(ModelLoadCMF(path))));
						Log::success("Mesh loaded: " + path);
					}
					else
					{
						Log::error("Can't load mesh: " + path);
					}
				}
			}
		}

		if (!serializer.getSubInt({"GameObjects", "Count"}, &count))
		{ Log::error("Can't load Scene Count: " + aFile); return false; }

		for (i = 0; i < static_cast<unsigned int>(count); i++)
		{
			std::string elem = "GameObject" + std::to_string(i);
			loadGameObject(&serializer, elem, i);
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
				if (Object.second->getName() == aName)
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

		if (mSkybox && mCamera)
		{
			mSkybox->setCamera(*mCamera);
		}

		for (auto Object : mObjects)
			Object.second->update();
	}
	//////////////////////////////////////////////////////////////////////////////
	void Scene::render()
	{
		C_EnableDepthTestOpenGL();
		C_EnableBlendOpenGL();
		C_EnableAlphaTestOpenGL();

		mNoneEffect.clearAttribs();
		mNoneEffect.addAttrib({ "uResolution", mContextSize });
		mNoneEffect.bind(Vector4(1, 1, 1, 0), mContextSize);
		
		if (mSkybox != nullptr)
			mSkybox->draw();

		for (auto Object : mObjects)
			if (Object.second->hasComponent("MeshRenderer"))
				Object.second->render();

		for (auto Object : mObjects)
			if (Object.second->hasComponent("ParticleSystem"))
				Object.second->render();

		mNoneEffect.unbind();

		mNoneEffect.draw();
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Scene::~Scene()
	{

	}

}




