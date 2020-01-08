#pragma once

#include <Scene/Transform.h>

namespace Columbus
{
	class GameObject;
	class Component
	{
	public:
		GameObject* gameObject = nullptr;
	public:
		enum class Type
		{
			Component,
			Light,
			MeshRenderer,
			MeshInstancedRenderer,
			ParticleSystem,
			Rigidbody,
			AudioSource,
			Billboard
		};
	public:
		Component() {}

		virtual void Update(float TimeTick) = 0;
		virtual void OnComponentAdd() {}

		virtual Type GetType() const { return Type::Component; }
		virtual void Destroy() {}

		virtual ~Component() {}
	};

}


