#include <Editor/CommonUI.h>
#include <Editor/Icons.h>
#include <Graphics/Device.h>
#include <Core/Filesystem.h>
#include <imgui_internal.h>

#include <Graphics/OpenGL/TextureOpenGL.h>

namespace Columbus::Editor
{
	_CommonUISettings CommonUISettings;

	void ApplyDarkTheme()
	{
		ImGuiStyle& Style = ImGui::GetStyle();
		ImGui::StyleColorsDark(&Style);
		Style.FrameRounding = 3.0f;
		Style.WindowRounding = 0.0f;
		Style.ScrollbarRounding = 3.0f;

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.75f, 0.75f, 0.75f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
	}

	void FlagButton(const char* name, bool& enabled)
	{
		if (ImGui::Button(name, enabled))
			enabled = !enabled;
	}

	bool ToolButton(const char* label, int* v, int v_button)
	{
		const bool pressed = ImGui::Button(label, *v == v_button);
		if (pressed)
			*v = v_button;

		return pressed;
	}

	void ToggleButton(const char* label, bool* v)
	{
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		float height = ImGui::GetFrameHeight();
		float width = height * 1.55f;
		float radius = height * 0.50f;

		ImGui::InvisibleButton(label, ImVec2(width, height));
		if (ImGui::IsItemClicked())
			*v = !*v;

		float t = *v ? 1.0f : 0.0f;

		ImGuiContext& g = *GImGui;
		float ANIM_SPEED = 0.08f;
		if (g.LastActiveId == g.CurrentWindow->GetID(label))// && g.LastActiveIdTimer < ANIM_SPEED)
		{
			float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
			t = *v ? (t_anim) : (1.0f - t_anim);
		}

		ImU32 col_bg;
		if (ImGui::IsItemHovered())
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
		else
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
		draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
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

	const char* GetFileIcon(const std::string& ext)
	{
		std::string e = str_tolower(ext);

		if (Filesystem::IsImage(ext)) return ICON_FA_FILE_IMAGE;
		if (e == "wav" || e == "mp3" || e == "ogg") return ICON_FA_MUSIC;
		if (e == "json" || e == "glsl" || e == "hlsl" || e == "csl") return ICON_FA_CODE;
		if (e == "hdr" || e == "exr") return ICON_FA_FILE_IMAGE;
		if (e == "scene") return ICON_FA_STRIKETHROUGH;
		if (e == "lig") return LIGHT_ICON;
		if (e == "mat") return MATERIAL_ICON;
		if (e == "par") return PARTICLES_ICON;
		if (e == "cmf" || e == "obj" || e == "dae" || e == "fbx") return MESH_ICON;
		if (e == "ttf") return ICON_FA_FONT;

		return ICON_FA_FILE;
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

		if (Size.x == 0) Size.x = Columbus::Editor::CommonUISettings.TreeNodeSize.x;
		if (Size.y == 0) Size.y = Columbus::Editor::CommonUISettings.TreeNodeSize.y;

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

	bool Button(const char* label, bool activated, const ImVec2& size)
	{
		auto active = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
		auto passive = ImGui::GetStyle().Colors[ImGuiCol_Button];

		ImGui::PushStyleColor(ImGuiCol_Button, activated ? active : passive);
		bool result = ImGui::Button(label, size);
		ImGui::PopStyleColor();

		return result;
	}

	void Image(Columbus::Texture* texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
	{
		switch (Columbus::gDevice->GetCurrentAPI())
		{
			case Columbus::GraphicsAPI::None: break;
			case Columbus::GraphicsAPI::OpenGL:
			{
				ImTextureID id = nullptr;
				if (texture)
					id = (ImTextureID)static_cast<Columbus::TextureOpenGL*>(texture)->GetID();

				Image(id, size, uv0, uv1, tint_col, border_col);
				break;
			}
		}
	}
}
