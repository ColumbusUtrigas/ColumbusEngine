#pragma once

#include <Core/ICloneable.h>
#include <Core/PrototypeFactory.h>

namespace Columbus::Editor
{

	class Extension : public ICloneable
	{
	public:
		Extension* Clone() const override { return nullptr; }
		virtual std::string_view GetTypename() const { return "Extension"; }

		// Path its a string like File/Save
		virtual std::string_view GetPath() const { return ""; }

		// This method will be called when you activate extension
		virtual void Execute() {}
	};

	#define DECLARE_EDITOR_EXTENSION(Type, Path) \
		DECLARE_PROTOTYPE(Extension, Type, #Type) \
		virtual std::string_view GetPath() const override { return Path; }

	#define IMPLEMENT_EDITOR_EXTENSION(Type) \
		IMPLEMENT_PROTOTYPE(Extension, Type)

}
