#include <Editor/Panel.h>
#include <Lib/imgui/imgui.h>

namespace Columbus
{

	void EditorPanel::Draw()
	{
		if (Opened)
		{
			ImGui::Begin(Name.c_str(), &Opened, ImGuiWindowFlags_NoCollapse);
			ImGui::End();
		}
	}

}


