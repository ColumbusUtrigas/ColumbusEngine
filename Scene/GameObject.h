#pragma once

#include <Graphics/Material.h>
#include <Scene/Transform.h>
#include <Scene/Component.h>
#include <System/Timer.h>
#include <Core/Containers/Array.h>
#include <Core/SmartPointer.h>
#include <vector>

namespace Columbus
{

	class GameObject
	{
	protected:
		std::vector<SmartPointer<GameObject>> Children;
		std::vector<SmartPointer<Component>> Components;

		std::string Name;

		Timer mTimer;

		Material ObjectMaterial;
	public:
		Transform transform;

		GameObject();
		GameObject(const GameObject&) = delete;
		GameObject(GameObject&&) = default;

		void SetName(std::string Name);
		std::string GetName() const;

		void AddChild(GameObject* Child);
		void AddComponent(Component* Component);

		void SetTransform(Transform Transform);
		Transform& GetTransform();

		void SetMaterial(Material InMaterial);
		Material& GetMaterial();

		void Update();
		void Render();

		bool HasComponent(Component::Type Type);
		Component* GetComponent(Component::Type Type);

		GameObject& operator=(const GameObject&) = delete;
		GameObject& operator=(GameObject&&) = default;

		~GameObject();
	};

}
