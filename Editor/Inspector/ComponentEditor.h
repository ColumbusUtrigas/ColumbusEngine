#pragma once

#include <Scene/Component.h>
#include <Core/ICloneable.h>
#include <Core/PrototypeFactory.h>

namespace Columbus::Editor
{

	class ComponentEditor : public ICloneable
	{
	protected:
		friend class PanelInspector;
		Component* Target;
	public:
		virtual void OnInspectorGUI() {}
		virtual std::string_view GetTypename() const { return "ComponentEditor"; }

		ComponentEditor* Clone() const override { return nullptr; }
	};

	#define DECLARE_COMPONENT_EDITOR(Type, Ed) \
		DECLARE_PROTOTYPE(ComponentEditor, Ed, Type::GetTypenameStatic())

	#define IMPLEMENT_COMPONENT_EDITOR(Type, Ed) \
		IMPLEMENT_PROTOTYPE(ComponentEditor, Ed)

}
