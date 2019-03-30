#include <Editor/PanelScene.h>
#include <Lib/imgui/imgui.h>

#include <Graphics/OpenGL/TextureOpenGL.h>

namespace Columbus
{

	void EditorPanelScene::Draw()
	{
		if (Opened)
		{
			ImVec2 Pos;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			if (ImGui::Begin(Name.c_str(), &Opened, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse))
			{
				Pos = ImGui::GetWindowPos();

				if (ImGui::BeginMenuBar())
				{
					ImGui::Checkbox("Stats", &Stats);
					ImGui::EndMenuBar();
				}

				Selected = ImGui::IsWindowFocused();
				Hover = ImGui::IsWindowHovered();

				auto Tex = ((TextureOpenGL*)FramebufferTexture)->GetID();
				auto Size = ImGui::GetWindowSize();
				Size.y -= ImGui::GetFrameHeight();
				ImGui::Image((void*)(intptr_t)Tex, ImVec2(Size.x, Size.y - 20), ImVec2(0, 1), ImVec2(1, 0));
				SizeOfRenderWindow = { (int)Size.x, (int)Size.y };

				//Pos.y += 20 + ImGui::GetFrameHeight();
				//ImGui::SetNextWindowPos(Pos);
				//ImGui::SetNextWindowSize(ImVec2(200, 150));
			}
			ImGui::End();
			ImGui::PopStyleVar(1);
		}
	}

}


