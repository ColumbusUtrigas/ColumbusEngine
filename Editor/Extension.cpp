#include <Editor/Extension.h>

namespace Columbus::Editor
{

	class TestExtension : public Extension
	{
		DECLARE_EDITOR_EXTENSION(TestExtension, "Extensions/Sub/Test")

		virtual void Execute() override
		{
			Log::Message(GetPath().data());
		}
	};
	IMPLEMENT_PROTOTYPE(Extension, TestExtension)

}
