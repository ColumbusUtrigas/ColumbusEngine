#pragma once
#include <imgui/imgui.h>
#include <Graphics/Texture.h>
#include <Core/String.h>

namespace Columbus::Editor
{

	struct _CommonUISettings
	{
		float TooltipDelay = 0.4f;
		ImVec2 TreeNodeSize = { -30, 20 };
	};
	extern _CommonUISettings CommonUISettings;

	void ApplyDarkTheme();
	void FlagButton(const char* name, bool& enabled, const char* tooltip = NULL);
	bool ToolButton(const char* label, int* v, int v_button, const ImVec2& size = {0,0}, const char* tooltip = NULL);
	void ToggleButton(const char* label, bool* v, const char* tooltip = NULL);
	void ShowTooltipDelayed(float delay, const char* fmt, ...);

	const char* GetFileIcon(const std::string& ext);

}

namespace ImGui
{
	void SetNextWindowPosCenter(ImGuiCond cond);
	bool TreeNodeSized(const char* label, ImVec2 size = {}, ImGuiTreeNodeFlags flags = 0);
	bool Button(const char* label, bool activated, bool hover_col_change = false, const ImVec2& size = {0,0});
	void Image(Columbus::Texture* texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
}
