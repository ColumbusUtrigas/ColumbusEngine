#include <Editor/CommonUI.h>
#include <imgui_internal.h>

namespace Columbus
{
	float CommonUISettings::TooltipDelay = 0.4f;

	void FlagButton(const char* name, bool& enabled)
	{
		auto active = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
		auto passive = ImGui::GetStyle().Colors[ImGuiCol_Button];

		ImGui::PushStyleColor(ImGuiCol_Button, enabled ? active : passive);
		ImGui::SameLine();
		if (ImGui::Button(name)) enabled = !enabled;
		ImGui::PopStyleColor();
	}

	void ShowTooltipDelayed(float delay, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > delay)
		{
			ImGui::SetTooltipV(fmt, args);
		}

		va_end(args);
	}
}
