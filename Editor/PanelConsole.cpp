#include <Editor/PanelConsole.h>
#include <Editor/FontAwesome.h>
#include <System/Log.h>
#include <Lib/imgui/imgui.h>

namespace Columbus
{

	void EditorPanelConsole::Draw()
	{
		if (Opened)
		{
			if (ImGui::Begin(ICON_FA_TERMINAL" Console##PanelConsole", &Opened, ImGuiWindowFlags_NoCollapse))
			{
				auto& list = Log::GetData();

				if (ImGui::Button("Clear##PanelConsole"))
					list.clear();

				ImGui::SameLine(); ImGui::Checkbox("I##PanelConsole", &_EnableInit);
				ImGui::SameLine(); ImGui::Checkbox("S##PanelConsole", &_EnableSuccess);
				ImGui::SameLine(); ImGui::Checkbox("M##PanelConsole", &_EnableMessage);
				ImGui::SameLine(); ImGui::Checkbox("W##PanelConsole", &_EnableWarning);
				ImGui::SameLine(); ImGui::Checkbox("E##PanelConsole", &_EnableError);
				ImGui::SameLine(); ImGui::Checkbox("F##PanelConsole", &_EnableFatal);

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
					ImGui::Selectable(text.c_str(), false);
				}
			}
			ImGui::End();
		}
	}

}

