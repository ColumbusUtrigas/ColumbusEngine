#include <Scene/GameObject.h>

namespace Columbus
{

	GameObject::GameObject()
	{

	}
	
	void GameObject::SetName(std::string Name)
	{
		this->Name = Name;
	}
	
	std::string GameObject::GetName() const
	{
		return Name;
	}
	
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
	
	void GameObject::Update()
	{
		for (auto& Comp : Components)
		{
			Comp->Update(mTimer.elapsed(), transform);
		}

		transform.Update();

		for (auto& Child : Children)
		{
			Child->Update();
		}

		mTimer.reset();
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

