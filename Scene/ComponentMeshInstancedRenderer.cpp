#include <Scene/ComponentMeshInstancedRenderer.h>

namespace Columbus
{

	ComponentMeshInstancedRenderer::ComponentMeshInstancedRenderer(MeshInstanced* InMesh) :
		Object(InMesh)
	{ }

	void ComponentMeshInstancedRenderer::Update(const float TimeTick)
	{
		if (Object != nullptr)
		{
			Object->SetCamera(ObjectCamera);
		}
	}

	void ComponentMeshInstancedRenderer::Render(Transform& InTransform)
	{
		if (Object != nullptr)
		{
			Object->Render();
		}
	}

	Component::Type ComponentMeshInstancedRenderer::GetType() const
	{
		return Component::Type::MeshInstancedRenderer;
	}

	void ComponentMeshInstancedRenderer::SetLights(std::vector<Light*> InLighs)
	{

	}

	void ComponentMeshInstancedRenderer::SetCamera(Camera InCamera)
	{
		ObjectCamera = InCamera;
	}

	ShaderProgram* ComponentMeshInstancedRenderer::GetShader() const
	{
		if (Object != nullptr)
		{
			return Object->Mat.GetShader();
		}

		return nullptr;
	}

	void ComponentMeshInstancedRenderer::SetShader(ShaderProgram* InShader)
	{
		if (Object != nullptr)
		{
			Object->Mat.SetShader(InShader);
		}
	}

	void ComponentMeshInstancedRenderer::SetReflection(Texture* Cubemap)
	{
		if (Object != nullptr)
		{
			Object->Mat.Reflection = Cubemap;
		}
	}

	ComponentMeshInstancedRenderer::~ComponentMeshInstancedRenderer() {}

}












