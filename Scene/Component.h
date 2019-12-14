#pragma once

#include <Scene/Transform.h>

namespace Columbus
{
	class GameObject;
	class Component
	{
	protected:
		friend class GameObject;
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
			AudioSource
		};
	public:
		Component() {}

		virtual void Update(float TimeTick, Transform& Trans) = 0;
		virtual void OnComponentAdd() {}

		virtual Type GetType() const { return Type::Component; }
		virtual void Destroy() {}

		virtual ~Component() {}
	};

}


