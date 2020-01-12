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

		Component* Clone() const final override
		{
			return new ComponentMeshRenderer(Object);
		}

		virtual void Update(float TimeTick) override {}
		virtual void OnComponentAdd() override
		{
			if (Object != nullptr)
			{
				bool first = true;
				for (auto& mat : Object->SubMeshes)
				{
					gameObject->materials.push_back(first ? gameObject->material : nullptr);
					first = false;
				}
			}
			else
			{
				gameObject->materials.push_back(gameObject->material);
			}
		}

		//This component methods
		virtual Type GetType() const override { return Component::Type::MeshRenderer; }
		Mesh*& GetMesh() { return Object; }

		virtual ~ComponentMeshRenderer() override {}
	};

}


