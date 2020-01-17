#include <Scene/GameObject.h>
#include <Core/Assert.h>

namespace Columbus
{

	GameObject::GameObject() {}
	
	GameObject* GameObject::AddChild(GameObject* Child)
	{
		COLUMBUS_ASSERT(Child != nullptr);
		if (Child->parent != nullptr)
			Child->parent->RemoveChild(Child);
		Child->parent = this;
		Children.push_back(Child);
		return Child;
	}

	void GameObject::RemoveChild(GameObject* Child)
	{
		Children.erase(std::remove(Children.begin(), Children.end(), Child));
	}
	
	Component* GameObject::AddComponent(Component* InComponent)
	{
		COLUMBUS_ASSERT(InComponent != nullptr);
		Components.push_back(SmartPointer<Component>(InComponent));
		Components.back()->gameObject = this;
		Components.back()->OnComponentAdd();
		return Components.back().Get();
	}
	
	void GameObject::Update(float DeltaTime)
	{
		if (Enable)
		{
			for (auto& Comp : Components)
			{
				Comp->Update(DeltaTime);
			}

			transform.Update();

			for (auto& Child : Children)
			{
				Child->Update(DeltaTime);
			}
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
		if (parent)
			parent->RemoveChild(this);

		for (auto& child : Children)
			delete child;
	}

}

