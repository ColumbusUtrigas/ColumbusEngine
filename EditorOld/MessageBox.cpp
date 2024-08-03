#include <Editor/MessageBox.h>
#include <Editor/CommonUI.h>
#include <Lib/imgui/imgui.h>

namespace Columbus::Editor
{

	int MessageBox::Draw()
	{
		if (Opened)
		{
			ImGui::OpenPopup(_name.c_str());
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(Size.X, Size.Y));
			if (ImGui::BeginPopupModal(_name.c_str(), &Opened, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
			{
				ImVec2 Size = ImGui::GetContentRegionAvail();
				if (ImGui::BeginChild("TextField##MessageBox", ImVec2(Size.x, Size.y - 30)))
				{
					ImGui::TextWrapped("%s", _text.c_str());
				}
				ImGui::EndChild();

				if (ImGui::BeginChild("Buttons##MessageBox"))
				{
					if (ImGui::Button("Cancel")) Selected = 1;
					ImGui::SameLine();
					if (ImGui::Button("No")) Selected = 2;
					ImGui::SameLine();
					if (ImGui::Button("Yes")) Selected = 3;
				}
				ImGui::EndChild();
				ImGui::EndPopup();

				if (Selected != 0) Close();
			}
		}

		return Selected;
	}

}


