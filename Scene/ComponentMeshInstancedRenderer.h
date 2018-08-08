#pragma once

#include <Scene/Component.h>
#include <Graphics/MeshInstanced.h>
#include <Graphics/Camera.h>
#include <Graphics/Shader.h>
#include <Graphics/Texture.h>

namespace Columbus
{

	class ComponentMeshInstancedRenderer : public Component
	{
	private:
		MeshInstanced* Object = nullptr;
		Camera ObjectCamera;
	public:
		ComponentMeshInstancedRenderer(MeshInstanced* InMesh);

		void Update(const float TimeTick) override;
		void Render(Transform& InTransform) override;
		//This component methods
		Type GetType() const override;
		void SetLights(std::vector<Light*> InLights);
		void SetCamera(Camera InCamera);
		ShaderProgram* GetShader() const;
		void SetShader(ShaderProgram* InShader);
		void SetReflection(Texture* Cubemap);

		~ComponentMeshInstancedRenderer() override;
	};

}







