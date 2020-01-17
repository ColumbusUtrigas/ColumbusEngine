#pragma once

#include <Scene/Transform.h>
#include <unordered_map>
#include <string_view>
#include <Core/Serialization.h>

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

		virtual Component* Clone() const = 0;

		virtual Type GetType() const { return Type::Component; }
		virtual std::string_view GetTypename() const = 0;
		virtual void Destroy() {}

		virtual ~Component() {}
	};

	struct ComponentFactory
	{
	private:
		std::unordered_map<std::string, Component*> _builders;
	public:
		static ComponentFactory& Instance()
		{
			static ComponentFactory f;
			return f;
		}

		auto GetBuilders() const
		{
			return _builders;
		}

		template <typename T, typename...Args>
		void Register(const std::string& type, Args...args)
		{
			auto tname = type;
			auto it = _builders.find(tname);
			if (it != _builders.end()) {
				Log::Error("Trying to re-registrate type: %s", tname);
			}
			_builders[tname] = new T(std::forward<Args...>(args)...);
		}

		Component* CreateFromTypename(const std::string& name)
		{
			auto it = _builders.find(name);
			if (it != _builders.end()) return it->second->Clone();
			return nullptr;
		}
	};

#define DECLARE_COMPONENT(Type, ...) \
	static struct Type##_ComponentTypeRegistrator { \
		Type##_ComponentTypeRegistrator() { \
			ComponentFactory::Instance().Register<Type>(#Type, __VA_ARGS__); \
		} \
	} Type##_ComponentTypeRegistrator_Init; \
	DECLARE_SERIALIZATION(Type); \
	public: \
		std::string_view GetTypename() const override { return #Type; }

#define IMPLEMENT_COMPONENT(Type) \
	Type::Type##_ComponentTypeRegistrator Type::Type##_ComponentTypeRegistrator_Init; \
	IMPLEMENT_SERIALIZATION(Type);

}


