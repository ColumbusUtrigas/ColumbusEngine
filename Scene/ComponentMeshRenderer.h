#pragma once

#include <Scene/Component.h>
#include <Graphics/Mesh.h>

namespace Columbus
{

	class ComponentMeshRenderer : public Component
	{
	private:
		Mesh* Object = nullptr;
	public:
		ComponentMeshRenderer(Mesh* InMesh) : Object(InMesh) {}

		virtual void Render(Transform& Transform) override {}
		virtual void Update(float TimeTick, Transform& Trans) override {}

		//This component methods
		virtual Type GetType() const override { return Component::Type::MeshRenderer; }
		Mesh* GetMesh() const { return Object; }
		void SetLights(std::vector<Light*> Lights) { Object->SetLights(Lights); }

		virtual ~ComponentMeshRenderer() override {}
	};

}



