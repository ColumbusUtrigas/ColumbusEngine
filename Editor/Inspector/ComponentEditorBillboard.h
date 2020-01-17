#include <Editor/Inspector/ComponentEditor.h>
#include <Scene/ComponentBillboard.h>
#include <imgui/imgui.h>

namespace Columbus
{

	class ComponentEditorBillboard : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentBillboard, ComponentEditorBillboard);
	public:
		void OnInspectorGUI() final override
		{
			auto& Bill = static_cast<ComponentBillboard*>(Target)->GetBillboard();

			const char* Types[] = { "Local X", "Local Y", "Local Z" };

			ImGui::Indent(10.0f);
			ImGui::Combo("Rotation##PanelInspector_Billboard", (int*)&Bill, Types, 3);
			ImGui::Unindent(10.0f);

			ImGui::Separator();
		}
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentBillboard, ComponentEditorBillboard);

}
