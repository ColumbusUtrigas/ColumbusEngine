#include <Scene/ComponentMeshRenderer.h>

namespace Columbus
{

	ComponentMeshRenderer::ComponentMeshRenderer(Mesh* InMesh) :
		Object(InMesh)
	{ }
	
	void ComponentMeshRenderer::Update(const float TimeTick)
	{
		if (Object != nullptr)
		{
			Object->SetCamera(ObjectCamera);
		}
	}

	void ComponentMeshRenderer::Render(Transform& Transform)
	{
		if (Object != nullptr)
		{
			//Object->Render(Transform);
		}
	}
	
	Component::Type ComponentMeshRenderer::GetType() const
	{
		return Component::Type::MeshRenderer;
	}

	Mesh* ComponentMeshRenderer::GetMesh() const
	{
		return Object;
	}
	
	void ComponentMeshRenderer::SetLights(std::vector<Light*> aLights)
	{
		Object->SetLights(aLights);
	}
	
	void ComponentMeshRenderer::SetCamera(Camera InCamera)
	{
		ObjectCamera = InCamera;
	}
	
	ShaderProgram* ComponentMeshRenderer::GetShader() const
	{
		if (Object != nullptr)
		{
			return Object->mMat.GetShader();
		}

		return nullptr;
	}
	
	void ComponentMeshRenderer::SetShader(ShaderProgram* Shader)
	{
		if (Object != nullptr)
		{
			Object->mMat.SetShader(Shader);
		}
	}
	
	void ComponentMeshRenderer::SetReflection(Cubemap* Cubemap)
	{
		if (Object != nullptr)
		{
			Object->mMat.setReflection(Cubemap);
		}
	}
	
	ComponentMeshRenderer::~ComponentMeshRenderer() { }

}


