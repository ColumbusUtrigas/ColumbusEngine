/************************************************
*                MeshRenderer.cpp               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   12.11.2017                  *
*************************************************/
#include <Scene/ComponentMeshRenderer.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	ComponentMeshRenderer::ComponentMeshRenderer(Mesh* Mesh) :
		mMesh(Mesh)
	{
		
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool ComponentMeshRenderer::OnCreate()
	{
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ComponentMeshRenderer::OnUpdate()
	{
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	void ComponentMeshRenderer::Update(const float TimeTick)
	{
		if (mMesh != nullptr)
		{
			mMesh->SetCamera(mCamera);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void ComponentMeshRenderer::Render(Transform& Transform)
	{
		if (mMesh != nullptr)
		{
			mMesh->Render(Transform);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	Component::Type ComponentMeshRenderer::GetType() const
	{
		return Component::Type::MeshRenderer;
	}
	//////////////////////////////////////////////////////////////////////////////
	void ComponentMeshRenderer::SetLights(std::vector<Light*> aLights)
	{
		mMesh->SetLights(aLights);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ComponentMeshRenderer::SetCamera(Camera Camera)
	{
		mCamera = Camera;
	}
	
	ShaderProgram* ComponentMeshRenderer::GetShader() const
	{
		if (mMesh == nullptr) return nullptr;
		return mMesh->mMat.GetShader();
	}
	
	void ComponentMeshRenderer::SetShader(ShaderProgram* Shader)
	{
		if (mMesh == nullptr) return;
		mMesh->mMat.SetShader(Shader);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ComponentMeshRenderer::SetReflection(Cubemap* Cubemap)
	{
		if (mMesh == nullptr) return;
		mMesh->mMat.setReflection(Cubemap);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	ComponentMeshRenderer::~ComponentMeshRenderer()
	{

	}

}


