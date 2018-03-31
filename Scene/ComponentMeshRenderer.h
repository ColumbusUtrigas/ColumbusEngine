/************************************************
*                 MeshRenderer.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   12.11.2017                  *
*************************************************/
#pragma once

#include <Scene/Component.h>
#include <Graphics/Mesh.h>
#include <Graphics/Camera.h>
#include <Graphics/Shader.h>
#include <Graphics/Cubemap.h>

namespace Columbus
{

	class ComponentMeshRenderer : public Component
	{
	private:
		bool CreateMoment = true;
		bool UpdateMoment = false;

		Mesh* mMesh = nullptr;
		Camera mCamera;
	public:
		ComponentMeshRenderer(Mesh* Mesh);

		bool OnCreate() override;
		bool OnUpdate() override;

		void Update(const float TimeTick) override;
		void Render(Transform& Transform) override;
		//This component methods
		Type GetType() const override;
		void SetLights(std::vector<Light*> Lights);
		void SetCamera(Camera Camera);
		Shader* GetShader() const;
		void SetShader(Shader* Shader);
		void SetReflection(Cubemap* Cubemap);

		~ComponentMeshRenderer() override;
	};

}



