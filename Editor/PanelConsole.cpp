#include <Editor/PanelConsole.h>
#include <Editor/FontAwesome.h>
#include <Editor/CommonUI.h>
#include <System/Log.h>
#include <Lib/imgui/imgui.h>

namespace Columbus::Editor
{

	static ImVec4 GetColor(Log::Type type)
	{
		switch (type)
		{
			case Log::Type_Initialization: return ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
			case Log::Type_Success:        return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
			case Log::Type_Message:        return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			case Log::Type_Warning:        return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
			case Log::Type_Error:          return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			case Log::Type_Fatal:          return ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
		}

		return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	PanelConsole::PanelConsole() : Panel(ICON_FA_TERMINAL" Console") {}
	PanelConsole::~PanelConsole() {}

	void PanelConsole::DrawInternal()
	{
		auto& list = Log::GetData();

		if (ImGui::BeginChild("##Header_PanelConsole", ImVec2(ImGui::GetContentRegionAvail().x, 25)))
		{
			if (ImGui::Button("Clear##PanelConsole"))
				list.clear();

			FlagButton(ICON_FA_INFO_CIRCLE"##PanelConsole", _EnableInit); ShowTooltipDelayed(CommonUISettings.TooltipDelay, "Init");
			FlagButton(ICON_FA_CHECK_CIRCLE"##PanelConsole", _EnableSuccess); ShowTooltipDelayed(CommonUISettings.TooltipDelay, "Success");
			FlagButton(ICON_FA_COMMENT"##PanelConsole", _EnableMessage); ShowTooltipDelayed(CommonUISettings.TooltipDelay, "Message");
			FlagButton(ICON_FA_EXCLAMATION_TRIANGLE"##PanelConsole", _EnableWarning); ShowTooltipDelayed(CommonUISettings.TooltipDelay, "Warning");
			FlagButton(ICON_FA_EXCLAMATION_CIRCLE"##PanelConsole", _EnableError); ShowTooltipDelayed(CommonUISettings.TooltipDelay, "Error");
			FlagButton(ICON_FA_CROSSHAIRS"##PanelConsole", _EnableFatal); ShowTooltipDelayed(CommonUISettings.TooltipDelay, "Fatal");
		}
		ImGui::EndChild();

		if (ImGui::BeginChild("##Header_PanelBody"))
		{
			size_t i = 0;
			for (const auto& elem : list)
			{
				if (elem.type == Log::Type_Initialization && !_EnableInit) continue;
				if (elem.type == Log::Type_Success && !_EnableSuccess) continue;
				if (elem.type == Log::Type_Message && !_EnableMessage) continue;
				if (elem.type == Log::Type_Warning && !_EnableWarning) continue;
				if (elem.type == Log::Type_Error && !_EnableError) continue;
				if (elem.type == Log::Type_Fatal && !_EnableFatal) continue;

				auto text = elem.text + "##PanelConsole_" + std::to_string(i++);

				ImGui::PushStyleColor(ImGuiCol_Text, GetColor(elem.type));
				ImGui::Selectable(text.c_str(), false);
				ImGui::PopStyleColor();

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", elem.text.c_str());
			}
		}

		if (_lastWasEnd && _lastWheel <= 0.0f && _lastCount < list.size())
			ImGui::SetScrollHereY(1.0f);

		_lastWasEnd = ImGui::GetScrollMaxY() == ImGui::GetScrollY();
		_lastWheel = ImGui::GetIO().MouseWheel;
		_lastCount = list.size();

		ImGui::EndChild();
	}

}

