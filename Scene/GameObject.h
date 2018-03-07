/************************************************
*              	   GameObejct.h                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   12.11.2017                  *
*************************************************/
#pragma once

#include <Graphics/Mesh.h>
#include <Scene/Transform.h>
#include <Scene/Component.h>
#include <System/Timer.h>

namespace Columbus
{

	class GameObject
	{
	protected:
		std::vector<GameObject*> mChildren;
		std::vector<Component*> mComponents;

		std::string mName;

		Timer mTimer;
	public:
		Transform transform;

		GameObject();

		void setName(const std::string aName);
		std::string getName() const;

		void addChild(GameObject* aChild);
		void addComponent(Component* aComponent);

		void setTransform(Transform aTransform);
		Transform getTransform() const;

		void update();
		void render();

		bool hasComponent(std::string aName);
		Component* getComponent(std::string aName);

		~GameObject();
	};

}
