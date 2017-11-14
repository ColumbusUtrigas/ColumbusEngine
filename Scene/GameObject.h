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

#include <Graphics/Model.h>
#include <Scene/Transform.h>
#include <Scene/Component.h>

namespace Columbus
{

	class C_GameObject
	{
	protected:
		std::vector<C_GameObject*> mChildren;
		std::vector<C_Component*> mComponents;

		C_Transform mTransform;
	public:
		C_GameObject();

		void addChild(C_GameObject* aChild);
		void addComponent(C_Component* aComponent);

		void setTransform(C_Transform aTransform);
		C_Transform getTransform() const;

		void update();
		void render();

		bool hasComponent(std::string aName);
		C_Component* getComponent(std::string aName);

		~C_GameObject();
	};

}
