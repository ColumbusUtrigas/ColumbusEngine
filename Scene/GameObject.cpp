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

	void GameObject::Update(float DeltaTime)
	{
		if (Enable)
		{
			transform.Update();

			for (auto& Child : Children)
			{
				Child->Update(DeltaTime);
			}
		}
	}
	
	GameObject::~GameObject()
	{
		if (parent)
			parent->RemoveChild(this);

		for (auto& child : Children)
			delete child;
	}

}

