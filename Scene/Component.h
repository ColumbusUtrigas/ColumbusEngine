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

namespace Columbus
{

	class C_Component
	{
	protected:
		virtual bool onCreate();

		virtual bool onUpdate();
	public:
		C_Component();

		virtual void update();

		virtual void render(C_Transform aTransform);

		void destroy();

		~C_Component();
	};

}


