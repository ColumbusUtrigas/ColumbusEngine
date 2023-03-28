#pragma once

#include <unordered_map>
#include <string_view>
#include <utility>
#include <type_traits>
#include <System/Log.h>

namespace Columbus
{

	template <typename T>
	class PrototypeFactory
	{
	private:
		std::unordered_map<std::string_view, T*> _builders;
	public:
		PrototypeFactory()
		{
			//static_assert(std::is_base_of<ICloneable, T>::value, __FUNCTION__": type parameter of this class must derive from ICloneable");
		}

		static PrototypeFactory& Instance()
		{
			static PrototypeFactory<T> f;
			return f;
		}

		auto GetBuilders() const
		{
			return _builders;
		}

		template <typename U, typename...Args>
		void Register(const std::string_view& type, Args...args)
		{
			auto tname = type;
			auto it = _builders.find(tname);
			if (it != _builders.end()) {
				Log::Error("Trying to re-register type: %s", tname);
			}
			_builders[tname] = new U(std::forward<Args>(args)...);
		}

		T* CreateFromTypename(const std::string_view& name)
		{
			auto it = _builders.find(name);
			if (it != _builders.end()) return static_cast<T*>(it->second->Clone());
			return nullptr;
		}

		T& GetFromTypename(const std::string_view& name)
		{
			auto it = _builders.find(name);
			if (it != _builders.end()) return *it->second;
			static T base;
			return base;
		}
	};

	#define DECLARE_PROTOTYPE(BaseType, Type, Typename, ...) \
		static struct Type##_PrototypeTypeRegistrator { \
			Type##_PrototypeTypeRegistrator() { \
				/*PrototypeFactory<BaseType>::Instance().Register<Type>(Typename, __VA_ARGS__);*/ \
			} \
		} Type##_PrototypeTypeRegistrator_Init; \
		/*DECLARE_SERIALIZATION(Type);*/ \
		public: \
			std::string_view GetTypename() const override { return Typename; } \
			static std::string_view GetTypenameStatic() { return Typename; }

	#define IMPLEMENT_PROTOTYPE(BaseType, Type) \
		Type::Type##_PrototypeTypeRegistrator Type::Type##_PrototypeTypeRegistrator_Init; \
		/*IMPLEMENT_SERIALIZATION(Type);*/

}
