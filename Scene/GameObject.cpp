#include <Scene/GameObject.h>

namespace Columbus
{

	GameObject::GameObject() {}
	
	void GameObject::AddChild(GameObject* Child)
	{
		Children.push_back(SmartPointer<GameObject>(Child));
	}
	
	void GameObject::AddComponent(Component* InComponent)
	{
		Components.push_back(SmartPointer<Component>(InComponent));
	}
	
	void GameObject::SetTransform(Transform Transform)
	{
		transform = Transform;
	}
	
	Transform& GameObject::GetTransform()
	{
		return transform;
	}

	void GameObject::SetMaterial(Material InMaterial)
	{
		ObjectMaterial = InMaterial;
	}

	Material& GameObject::GetMaterial()
	{
		return ObjectMaterial;
	}
	
	void GameObject::Update(float DeltaTime)
	{
		for (auto& Comp : Components)
		{
			Comp->Update(DeltaTime, transform);
		}

		transform.Update();

		for (auto& Child : Children)
		{
			Child->Update(DeltaTime);
		}
	}
	
	void GameObject::Render()
	{
		for (auto& Comp : Components)
		{
			Comp->Render(transform);
		}

		for (auto& Child : Children)
		{
			Child->Render();
		}
	}
	
	bool GameObject::HasComponent(Component::Type Type)
	{
		for (const auto& Comp : Components)
		{
			if (Comp->GetType() == Type)
			{
				return true;
			}
		}

		return false;
	}
	
	Component* GameObject::GetComponent(Component::Type Type)
	{
		for (const auto& Comp : Components)
		{
			if (Comp->GetType() == Type)
			{
				return Comp.Get();
			}
		}

		return nullptr;
	}
	
	GameObject::~GameObject()
	{

	}

}

