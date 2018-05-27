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
		Mesh* Object = nullptr;
		Camera ObjectCamera;
	public:
		ComponentMeshRenderer(Mesh* InMesh);

		void Update(const float TimeTick) override;
		void Render(Transform& Transform) override;
		//This component methods
		Type GetType() const override;
		void SetLights(std::vector<Light*> Lights);
		void SetCamera(Camera InCamera);
		ShaderProgram* GetShader() const;
		void SetShader(ShaderProgram* Shader);
		void SetReflection(Cubemap* Cubemap);

		~ComponentMeshRenderer() override;
	};

}



