#pragma once

#include <Graphics/Material.h>
#include <Scene/Transform.h>
#include <Scene/Component.h>
#include <Core/SmartPointer.h>
#include <vector>

namespace Columbus
{

	class GameObject
	{
	protected:
		std::vector<SmartPointer<GameObject>> Children;
		std::vector<SmartPointer<Component>> Components;

		Material ObjectMaterial;
	public:
		Transform transform;
		std::string Name;
	public:
		GameObject();
		GameObject(const GameObject&) = delete;
		GameObject(GameObject&&) = default;

		void AddChild(GameObject* Child);
		void AddComponent(Component* Component);

		void SetTransform(Transform Transform);
		Transform& GetTransform();

		void SetMaterial(Material InMaterial);
		Material& GetMaterial();

		void Update(float DeltaTime);
		void Render();

		template <typename T>
		T* GetComponent()
		{
			for (const auto& Comp : Components)
			{
				T* C = dynamic_cast<T*>(Comp.Get());
				if (C != nullptr) return C;
			}

			return nullptr;
		}

		bool HasComponent(Component::Type Type);
		Component* GetComponent(Component::Type Type);

		GameObject& operator=(const GameObject&) = delete;
		GameObject& operator=(GameObject&&) = default;

		~GameObject();
	};

}


