#include <Editor/Inspector/ComponentEditor.h>
#include <Scene/ComponentBillboard.h>
#include <imgui/imgui.h>

namespace Columbus::Editor
{

	class ComponentEditorBillboard : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentBillboard, ComponentEditorBillboard);
	public:
		void OnInspectorGUI() final override
		{
			auto& Bill = static_cast<ComponentBillboard*>(Target)->GetBillboard();

			const char* Types[] = { "Local X", "Local Y", "Local Z" };

			ImGui::Combo("Rotation", (int*)&Bill, Types, 3);
		}
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentBillboard, ComponentEditorBillboard);

}
