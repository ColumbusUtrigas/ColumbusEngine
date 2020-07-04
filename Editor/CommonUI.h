#pragma once
#include <imgui/imgui.h>
#include <Core/String.h>

namespace Columbus
{

	struct _CommonUISettings
	{
		float TooltipDelay = 0.4f;
		ImVec2 TreeNodeSize = { -30, 20 };
	};
	extern _CommonUISettings CommonUISettings;

	void FlagButton(const char* name, bool& enabled);
	void ShowTooltipDelayed(float delay, const char* fmt, ...);

	const char* GetFileIcon(const std::string& ext);

}

namespace ImGui
{
	void SetNextWindowPosCenter(ImGuiCond cond);
	bool TreeNodeSized(const char* label, ImVec2 size = {}, ImGuiTreeNodeFlags flags = 0);
}
