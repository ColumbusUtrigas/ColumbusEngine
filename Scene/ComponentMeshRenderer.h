#pragma once

#include <Scene/Component.h>
#include <Graphics/Mesh.h>
#include <Graphics/Camera.h>
#include <Graphics/Shader.h>
#include <Graphics/Texture.h>

namespace Columbus
{

	class ComponentMeshRenderer : public Component
	{
	private:
		Mesh* Object = nullptr;
		Camera ObjectCamera;
	public:
		ComponentMeshRenderer(Mesh* InMesh) : Object(InMesh) {}

		virtual void Render(Transform& Transform) override {}
		virtual void Update(float TimeTick, Transform& Trans) override
		{
			if (Object != nullptr)
			{
				Object->SetCamera(ObjectCamera);
			}
		}

		//This component methods
		virtual Type GetType() const override { return Component::Type::MeshRenderer; }
		Mesh* GetMesh() const { return Object; }
		void SetLights(std::vector<Light*> Lights) { Object->SetLights(Lights); }
		void SetCamera(Camera InCamera) { ObjectCamera = InCamera; }

		virtual ~ComponentMeshRenderer() override {}
	};

}



