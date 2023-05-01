#pragma once

#include <Graphics/Material.h>
#include <Scene/Transform.h>
#include <Core/SmartPointer.h>
#include <Core/String.h>
#include <vector>

namespace Columbus
{

	class GameObject
	{
	protected:
		GameObject* parent = nullptr;
		std::vector<GameObject*> Children;
	public:
		Transform transform;
		Material* material = nullptr;
		std::vector<Material*> materials;

		String Name;
		bool Enable = true;
	public:
		GameObject();
		GameObject(const GameObject&) = delete;
		GameObject(GameObject&&) = default;

		GameObject* AddChild(GameObject* Child);
		void RemoveChild(GameObject* Child);

		GameObject* Clone() const
		{
			auto n = new GameObject();
			n->transform = transform;
			n->Name = Name;
			n->Enable = Enable;

			for (const auto& child : Children)
				n->AddChild(child->Clone());

			n->material = material;
			n->materials = materials;

			return n;
		}

		void Update(float DeltaTime);

		const auto& GetChildren() const { return Children; }

		GameObject* GetParent() const { return parent; }

		GameObject& operator=(const GameObject&) = delete;
		GameObject& operator=(GameObject&&) = default;

		~GameObject();
	};

}


