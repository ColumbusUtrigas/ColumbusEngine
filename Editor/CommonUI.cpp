#include <Editor/CommonUI.h>

namespace Columbus
{
	void FlagButton(const char* name, bool& enabled)
	{
		auto active = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
		auto passive = ImGui::GetStyle().Colors[ImGuiCol_Button];

		ImGui::PushStyleColor(ImGuiCol_Button, enabled ? active : passive);
		ImGui::SameLine();
		if (ImGui::Button(name)) enabled = !enabled;
		ImGui::PopStyleColor();
	}
}
