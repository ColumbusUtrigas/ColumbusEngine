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

namespace Columbus
{

	C_Scene::C_Scene() :
		mSkybox(nullptr)
	{
		mNoneShader = new C_Shader("Data/Shaders/post.vert", "Data/Shaders/NonePost.frag");

		mNoneEffect.setShader(mNoneShader);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::lightWorkflow()
	{
		mLights.clear();
		
		for (auto Mesh : mMeshes)
		{
			C_LightComponent* light =
				static_cast<C_LightComponent*>(Mesh.second->getComponent("LightComponent"));

			if (light != nullptr)
				mLights.push_back(light->getLight());
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::meshWorkflow()
	{
		for (auto Mesh : mMeshes)
		{
			C_MeshRenderer* mesh =
				static_cast<C_MeshRenderer*>(Mesh.second->getComponent("MeshRenderer"));

			if (mesh != nullptr)
			{
				mesh->setLights(mLights);
				if (mSkybox != nullptr)
					mesh->setReflection(mSkybox->getCubemap());
				if (mCamera != nullptr)
					mesh->setCamera(*mCamera);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::particlesWorkflow()
	{
		for (auto PS : mMeshes)
		{
			C_ParticleSystem* ps =
				static_cast<C_ParticleSystem*>(PS.second->getComponent("ParticleSystem"));

			if (ps != nullptr)
			{
				ps->setLights(mLights);
				if (mCamera != nullptr)
					ps->setCamera(*mCamera);
			}
		}
	}
	bool C_Scene::loadGameObject(Serializer::C_SerializerXML* aSerializer,
		std::string aElement, unsigned int aID)
	{
		std::string name;
		std::string materialPath;
		std::string vertShaderPath;
		std::string fragShaderPath;
		std::string meshPath;
		std::string particlePath;
		std::string lightPath;

		C_GameObject* GameObject = new C_GameObject();
		C_Transform Transform;
		C_Vector3 position;
		C_Vector3 rotation;
		C_Vector3 scale;
		C_Material* material = new C_Material();
		C_Shader* shader = new C_Shader();
		Import::C_ImporterModel imp;

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

		if (!aSerializer->getSubString({"GameObjects", aElement, "ShaderVertex"}, &vertShaderPath)) return false;  // TO
		if (!aSerializer->getSubString({"GameObjects", aElement, "ShaderFragment"}, &fragShaderPath)) return false;// DO

		shader->load(vertShaderPath, fragShaderPath);
		material->setShader(shader);

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
				GameObject->addComponent(new C_MeshRenderer(new C_Mesh(C_PrimitivePlane(), *material)));
			} else if (meshPath == "Cube")
			{
				GameObject->addComponent(new C_MeshRenderer(new C_Mesh(C_PrimitiveBox(), *material)));
			} else if (meshPath == "Sphere")
			{
				GameObject->addComponent(new C_MeshRenderer(new C_Mesh(C_PrimitiveSphere(1, 50, 50), *material)));
			} else
			{
				imp.loadOBJ(meshPath);
				if (imp.getCount() > 0)
					GameObject->addComponent(new C_MeshRenderer(new C_Mesh(imp.getObject(0), *material)));
			}
		}

		if (aSerializer->getSubString({"GameObjects", aElement, "Components", "ParticleSystem", "Particles"}, &particlePath))
		{
			GameObject->addComponent(new C_ParticleSystem(new C_ParticleEmitter(new C_ParticleEffect(particlePath, material))));
		}

		if (aSerializer->getSubString({"GameObjects", aElement, "Components", "LightComponent", "Light"}, &lightPath))
		{
			GameObject->addComponent(new C_LightComponent(new C_Light(lightPath, position)));
		}

		Transform.setPos(position);
		Transform.setRot(rotation);
		Transform.setScale(scale);
		GameObject->setTransform(Transform);

		add(aID, GameObject);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool C_Scene::load(std::string aFile)
	{
		Serializer::C_SerializerXML serializer;

		if (!serializer.read(aFile, "Scene"))
		{ C_Log::error("Can't load Scene: " + aFile); return false; }

		int count = 0;
		int texCount = 0;

		if (serializer.getSubInt({"Resources", "Textures", "Count"}, &texCount))
		{
			for (int i = 0; i < texCount; i++)
			{
				std::string path;
				std::string elem = std::string("Texture") + std::to_string(i);
				if (serializer.getSubString({"Resources", "Textures", elem}, &path))
					mTextures.insert(std::pair<int, C_Texture*>(i, new C_Texture(path)));
			}
		}

		if (!serializer.getSubInt({"GameObjects", "Count"}, &count))
		{ C_Log::error("Can't load Scene Count: " + aFile); return false; }

		for (int i = 0; i < count; i++)
		{
			std::string elem = "GameObject" + std::to_string(i);
			loadGameObject(&serializer, elem, i);
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::add(unsigned int aID, C_GameObject* aMesh)
	{
		mMeshes.insert(std::pair<unsigned int, C_GameObject*>(aID, aMesh));
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::setSkybox(const C_Skybox* aSkybox)
	{
		mSkybox = const_cast<C_Skybox*>(aSkybox);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::setCamera(const C_Camera* aCamera)
	{
		mCamera = const_cast<C_Camera*>(aCamera);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::setContextSize(const C_Vector2 aContextSize)
	{
		mContextSize = static_cast<C_Vector2>(aContextSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::update()
	{
		lightWorkflow();
		meshWorkflow();
		particlesWorkflow();

		for (auto Mesh : mMeshes)
			Mesh.second->update();
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Scene::render()
	{
		C_EnableDepthTestOpenGL();
		C_EnableBlendOpenGL();
		C_EnableAlphaTestOpenGL();

		mNoneEffect.bind(C_Vector4(1, 1, 1, 0), mContextSize);
		
		if (mSkybox != nullptr)
			mSkybox->draw();

		for (auto Object : mMeshes)
			if (Object.second->hasComponent("MeshRenderer"))
				C_Render::render(Object.second);

		for (auto Object : mMeshes)
			if (Object.second->hasComponent("ParticleSystem"))
				C_Render::render(Object.second);

		mNoneEffect.unbind();

		mNoneEffect.draw();
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Scene::~C_Scene()
	{

	}

}




