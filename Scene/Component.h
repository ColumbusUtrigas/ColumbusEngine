#pragma once

#include <Scene/Transform.h>
#include <string>

namespace Columbus
{

	class Component
	{
	public:
		enum class Type
		{
			Component,
			Light,
			MeshRenderer,
			MeshInstancedRenderer,
			ParticleSystem,
			Rigidbody
		};
	public:
		Component() {}

		virtual void Update(const float TimeTick) {}
		virtual void Render(Transform& Trans) {}
		virtual Type GetType() const { return Type::Component; }
		virtual void Destroy() {}

		virtual ~Component() {}
	};

}



