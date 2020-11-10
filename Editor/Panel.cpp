#include <Editor/Panel.h>
#include <imgui/imgui.h>

namespace Columbus::Editor
{

	void Panel::Draw()
	{
		if (Opened)
		{
			bool pad = _padding;
			bool pad_pop = false;

			ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
			flags |= _menu ? ImGuiWindowFlags_MenuBar : 0;

			if (!pad)
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

			ImGui::PushID(_name.data());
			if (ImGui::Begin(_name.data(), &Opened, flags))
			{
				if (!pad)
				{
					ImGui::PopStyleVar();
					pad_pop = true;
				}

				DrawInternal();
			}
			ImGui::End();
			ImGui::PopID();

			if (!pad && !pad_pop)
			{
				ImGui::PopStyleVar();
			}
		}
	}

}
