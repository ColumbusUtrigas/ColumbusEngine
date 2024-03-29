#pragma once

#include <Graphics/Material.h>
#include <Scene/Transform.h>
#include <Scene/Component.h>
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
		std::vector<SmartPointer<Component>> Components;
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
		Component* AddComponent(Component* Component);

		template <typename T, typename...Args>
		T* AddComponent(Args&&...args)
		{
			return static_cast<T*>(AddComponent(new T(std::forward<Args>(args)...)));
		}

		GameObject* Clone() const
		{
			auto n = new GameObject();
			n->transform = transform;
			n->Name = Name;
			n->Enable = Enable;

			for (const auto& comp : Components)
				n->AddComponent(comp->Clone());

			for (const auto& child : Children)
				n->AddChild(child->Clone());

			n->material = material;
			n->materials = materials;

			return n;
		}

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

		const auto& GetComponents() const { return Components; }

		template <typename T>
		bool HasComponent()
		{
			return GetComponent<T>() != nullptr;
		}

		const auto& GetChildren() const { return Children; }

		GameObject* GetParent() const { return parent; }

		/*template <typename T>
		bool DeleteComponent()
		{
			if (GetComponent<T>() != nullptr)
			{
				auto Pred = [](const SmartPointer<Component>& A)
				{
					return dynamic_cast<T*>(A.Get()) != nullptr;
				};
				auto Removed = std::remove_if(Components.begin(), Components.end(), Pred);
				Components.erase(Removed, Components.end());

				return true;
			}

			return false;
		}*/

		bool HasComponent(Component::Type Type);
		Component* GetComponent(Component::Type Type);

		GameObject& operator=(const GameObject&) = delete;
		GameObject& operator=(GameObject&&) = default;

		~GameObject();
	};

}


