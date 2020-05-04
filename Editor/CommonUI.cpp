#include <Editor/CommonUI.h>
#include <imgui_internal.h>

namespace Columbus
{
	_CommonUISettings CommonUISettings;

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

namespace ImGui
{
	void SetNextWindowPosCenter(ImGuiCond cond)
	{
		auto DisplaySize = ImGui::GetIO().DisplaySize;
		auto Center = ImVec2(DisplaySize.x / 2, DisplaySize.y / 2);
		ImGui::SetNextWindowPos(Center, cond, ImVec2(0.5f, 0.5f));
	}

	bool TreeNodeSized(const char* label, ImVec2 size, ImGuiTreeNodeFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		float LineSizeY = window->DC.CurrLineSize.y;
		float LineBaseOffset = window->DC.CurrLineTextBaseOffset;

		ImVec2 Size = size;

		if (Size.x == 0) Size.x = Columbus::CommonUISettings.TreeNodeSize.x;
		if (Size.y == 0) Size.y = Columbus::CommonUISettings.TreeNodeSize.y;

		if (Size.x < 0) Size.x += GetContentRegionAvail().x;
		if (Size.y < 0) Size.y += GetContentRegionAvail().y;

		bool Open = false;
		if (ImGui::BeginChild("TreeNode", Size))
		{
			window = GetCurrentWindow();
			window->DC.CurrLineSize.y = LineSizeY;
			window->DC.CurrLineTextBaseOffset = LineBaseOffset;

			Open = ImGui::TreeNodeEx(label, flags);
		}
		ImGui::EndChild();

		return Open;
	}
}
