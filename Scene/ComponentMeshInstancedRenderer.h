#pragma once

#include <Scene/Component.h>
#include <Graphics/MeshInstanced.h>
#include <Graphics/Camera.h>
#include <Graphics/Shader.h>
#include <Graphics/Cubemap.h>

namespace Columbus
{

	class ComponentMeshInstancedRenderer : public Component
	{
	private:
		bool CreateMoment = true;
		bool UpdateMoment = false;

		MeshInstanced* Object = nullptr;
		Camera ObjectCamera;
	public:
		ComponentMeshInstancedRenderer(MeshInstanced* InMesh);

		bool OnCreate() override;
		bool OnUpdate() override;

		void Update(const float TimeTick) override;
		void Render(Transform& InTransform) override;
		//This component methods
		Type GetType() const override;
		void SetLights(std::vector<Light*> InLights);
		void SetCamera(Camera InCamera);
		ShaderProgram* GetShader() const;
		void SetShader(ShaderProgram* InShader);
		void SetReflection(Cubemap* InCubemap);

		~ComponentMeshInstancedRenderer() override;
	};

}







