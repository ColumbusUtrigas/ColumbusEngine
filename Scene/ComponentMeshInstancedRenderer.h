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
		ComponentMeshInstancedRenderer(MeshInstanced* InMesh) : Object(InMesh) {}

		virtual void Render(Transform& InTransform) override {}
		virtual void Update(float TimeTick, Transform& Trans) override
		{
			if (Object != nullptr)
			{
				Object->SetCamera(ObjectCamera);
			}
		}

		//This component methods
		virtual Type GetType() const override { return Component::Type::MeshInstancedRenderer; }
		void SetLights(std::vector<Light*> InLights) {}
		void SetCamera(const Camera& InCamera) { ObjectCamera = InCamera; }
		ShaderProgram* GetShader() const
		{
			if (Object != nullptr)
			{
				return Object->Mat.GetShader();
			}

			return nullptr;
		}

		void SetShader(ShaderProgram* InShader)
		{
			if (Object != nullptr)
			{
				Object->Mat.SetShader(InShader);
			}
		}

		void SetReflection(Texture* Cubemap)
		{
			if (Object != nullptr)
			{
				Object->Mat.Reflection = Cubemap;
			}
		}

		virtual ~ComponentMeshInstancedRenderer() override {}
	};

}







