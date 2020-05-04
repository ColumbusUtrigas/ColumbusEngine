#include <Editor/Inspector/ComponentEditor.h>
#include <Scene/ComponentRigidbody.h>

namespace Columbus
{

	class ComponentEditorRigidbody : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentRigidbody, ComponentEditorRigidbody);
	public:
		void OnInspectorGUI() final override;
	};

}
