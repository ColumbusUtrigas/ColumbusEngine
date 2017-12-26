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
	//////////////////////////////////////////////////////////////////////////////
	bool C_Scene::load(std::string aFile)
	{
		Serializer::C_SerializerXML serializer;

		if (!serializer.read(aFile, "Scene"))
		{ C_Log::error("Can't load Scene: " + aFile); return false; }

		int count = 0;

		if (!serializer.getInt("Count", &count))
		{ C_Log::error("Can't load Scene Count: " + aFile); return false; }

		for (int i = 0; i < count; i++)
		{
			std::string name;
			std::string materialPath;
			std::string vertShaderPath;
			std::string fragShaderPath;
			std::string meshPath;

			std::string elem = std::string("GameObject") + std::to_string(i);

			C_GameObject* GameObject = new C_GameObject();
			C_Transform Transform;
			C_Vector3 position;
			C_Material* material = new C_Material();
			C_Shader* shader = new C_Shader();
			Import::C_ImporterModel imp;

			if (!serializer.getSubString({elem, "Name"}, &name)) continue;

			if (!serializer.getSubVector3({elem, "Transform", "Position"}, &position, {"X", "Y", "Z"}))
			{ C_Log::error("Can't load Scene Object Material: " + aFile); return false; }

			if (!serializer.getSubString({elem, "Material"}, &materialPath))
			{ C_Log::error("Can't load Scene Object Material: " + aFile); return false; }

			material->loadFromXML(materialPath);

			if (!serializer.getSubString({elem, "ShaderVertex"}, &vertShaderPath))
			{ C_Log::error("Can't load Scene Object Vertex Shader: " + aFile); return false; }

			if (!serializer.getSubString({elem, "ShaderFragment"}, &fragShaderPath))
			{ C_Log::error("Can't load Scene Object Fragment Shader: " + aFile); return false; }

			shader->load(vertShaderPath, fragShaderPath);

			if (!serializer.getSubString({elem, "MeshRenderer"}, &meshPath))
			{ C_Log::error("Can't load Scene Object Mesh: " + aFile); return false; }

			imp.loadOBJ(meshPath);
			C_Mesh* mesh = new C_Mesh(imp.getObject(0));
			material->setShader(shader);
			mesh->mMat = *material;

			Transform.setPos(position);
			GameObject->setTransform(Transform);
			GameObject->addComponent(new C_MeshRenderer(mesh));

			add(i, GameObject);
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




