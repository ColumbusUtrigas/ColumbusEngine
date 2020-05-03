#pragma once

#include <Scene/Transform.h>
#include <unordered_map>
#include <string_view>
#include <Core/ICloneable.h>
#include <Core/Serialization.h>
#include <Core/PrototypeFactory.h>

namespace Columbus
{

	class GameObject;
	class Component : public ICloneable
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

		virtual Component* Clone() const override = 0;

		virtual Type GetType() const { return Type::Component; }
		virtual std::string_view GetTypename() const = 0;
		virtual void Destroy() {}

		virtual ~Component() {}
	};

#define DECLARE_COMPONENT(Type, ...) \
	DECLARE_PROTOTYPE(Component, Type, #Type, __VA_ARGS__)

#define IMPLEMENT_COMPONENT(Type) \
	IMPLEMENT_PROTOTYPE(Component, Type)

}


