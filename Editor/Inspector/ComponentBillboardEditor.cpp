#include <Editor/Inspector/PanelInspector.h>
#include <Scene/ComponentBillboard.h>
#include <Lib/imgui/imgui.h>

namespace Columbus
{

	void EditorPanelInspector::DrawComponentBillboardEditor(Scene& Scn)
	{
		auto Co = Inspectable->GetComponent<ComponentBillboard>();

		if (Co != nullptr)
		{
			if (ImGui::CollapsingHeader("Billboard##PanelInspector_Billboard"))
			{
				auto& Bill = Co->GetBillboard();

				const char* Types[] = { "Local X", "Local Y", "Local Z" };

				ImGui::Indent(10.0f);
				ImGui::Combo("Rotation##PanelInspector_Billboard", (int*)&Bill, Types, 3);
				ImGui::Unindent(10.0f);

				ImGui::Separator();
			}
		}
	}

}
