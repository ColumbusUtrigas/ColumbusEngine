#include <Scene/GameObject.h>
#include <System/Assert.h>

namespace Columbus
{

	GameObject::GameObject() {}
	
	void GameObject::AddChild(GameObject* Child)
	{
		COLUMBUS_ASSERT(Child != nullptr);
		Children.push_back(SmartPointer<GameObject>(Child));
	}
	
	void GameObject::AddComponent(Component* InComponent)
	{
		COLUMBUS_ASSERT(InComponent != nullptr);
		Components.push_back(SmartPointer<Component>(InComponent));
	}
	
	void GameObject::Update(float DeltaTime)
	{
		if (Enable)
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

