#include <Editor/Menu.h>
#include <imgui.h>

namespace Columbus::Editor
{
	void MenuItem::Draw()
	{
		if (children.size() != 0)
		{
			if (ImGui::BeginMenu(title.c_str()))
			{
				for (auto& item : children)
				{
					ImGui::Spacing();
					item.second.Draw();
				}

				ImGui::Spacing();
				ImGui::EndMenu();
			}
		}
		else
		{
			if (ImGui::MenuItem(title.c_str(), shortcut.data(), selected))
				if (onExecute) onExecute();
		}
	}

	void MenuBar::Draw()
	{
		if (ImGui::BeginMenuBar())
		{
			for (auto& menu : menus)
			{
				menu.second.Draw();
			}
			ImGui::EndMenuBar();
		}
	}
}
