#pragma once

#include <Scene/Component.h>
#include <unordered_map>
#include <string_view>

namespace Columbus
{

	class ComponentEditor
	{
	protected:
		friend class EditorPanelInspector;
		Component* Target;
	public:
		virtual void OnInspectorGUI() {}
	};

	struct ComponentEditorDatabase
	{
	private:
		std::unordered_map<std::string_view, ComponentEditor*> _builders;
	public:
		static ComponentEditorDatabase& Instance()
		{
			static ComponentEditorDatabase db;
			return db;
		}

		template <typename T>
		void Register(std::string_view type)
		{
			auto tname = type;
			auto it = _builders.find(tname);
			if (it != _builders.end()) {
				Log::Error("Trying to re-registrate type editor: %s", tname);
			}
			_builders[tname] = new T();
		}

		ComponentEditor& GetFromTypename(std::string_view name)
		{
			auto it = _builders.find(name);
			if (it != _builders.end()) return *it->second;
			static ComponentEditor ed;
			return ed;
		}
	};

#define DECLARE_COMPONENT_EDITOR(Type, Ed) \
	static struct Type##_ComponentEditorTypeRegistrator { \
		Type##_ComponentEditorTypeRegistrator() { \
			ComponentEditorDatabase::Instance().Register<Ed>(#Type); \
		} \
	} Type##_ComponentEditorTypeRegistrator_Init;

#define IMPLEMENT_COMPONENT_EDITOR(Type, Ed) \
	Ed::Type##_ComponentEditorTypeRegistrator Ed::Type##_ComponentEditorTypeRegistrator_Init;

}
