#include <Editor/PanelHierarchy.h>
#include <Lib/imgui/imgui.h>

namespace Columbus
{

	void EditorPanelHierarchy::Draw()
	{
		if (Opened && scene != nullptr)
		{
			if (ImGui::Begin(Name.c_str(), &Opened, ImGuiWindowFlags_NoCollapse))
			{
				for (auto& Obj : scene->Objects)
				{
					if (ImGui::Selectable(Obj.second->Name.c_str(), object == Obj.second.Get()))
					{
						object = Obj.second.Get();
					}
				}
			}
			ImGui::End();
		}
	}

}


