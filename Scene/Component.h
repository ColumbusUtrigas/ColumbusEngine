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
	public:
		enum Type
		{
			COMPONENT,
			COMPONENT_LIGHT,
			COMPONENT_MESH_RENDERER,
			COMPONENT_PARTICLE_SYSTEM,
			COMPONENT_RIGIDBODY
		};
	public:
		Component();

		virtual bool OnCreate();
		virtual bool OnUpdate();

		virtual void Update(const float aTimeTick);
		virtual void Render(Transform& aTransform);
		virtual Type GetType() const;
		virtual void Destroy();

		virtual ~Component();
	};

}



