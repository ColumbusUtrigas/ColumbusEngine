/************************************************
*                  Component.h                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   12.11.2017                  *
*************************************************/
#pragma once

#include <Scene/Transform.h>
#include <string>

namespace Columbus
{

	class Component
	{
	protected:
		virtual bool onCreate();

		virtual bool onUpdate();
	public:
		Component();

		virtual void update(const float aTimeTick);
		virtual void render(Transform& aTransform);
		virtual std::string getType();
		virtual void destroy();

		virtual ~Component();
	};

}



