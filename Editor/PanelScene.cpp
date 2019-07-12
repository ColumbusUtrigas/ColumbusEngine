#include <Editor/PanelScene.h>
#include <Editor/FontAwesome.h>
#include <Lib/imgui/imgui.h>
#include <Graphics/OpenGL/TextureOpenGL.h>

namespace Columbus
{

	void EditorPanelScene::Draw(Renderer& Render)
	{
		if (Opened)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			if (ImGui::Begin(ICON_FA_GLOBE" Scene", &Opened, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse))
			{
				if (ImGui::BeginMenuBar())
				{
					ImGui::Checkbox("Icons##PanelScene", &Render.DrawIcons);
					ImGui::EndMenuBar();
				}

				Selected = ImGui::IsWindowFocused();
				Hover = ImGui::IsWindowHovered();

				Position = iVector2((int)ImGui::GetCursorScreenPos().x, (int)ImGui::GetCursorScreenPos().y);

				auto Tex = ((TextureOpenGL*)FramebufferTexture)->GetID();
				auto Cursor = ImGui::GetCursorPos();
				auto Size = ImGui::GetWindowSize();
				Size.y -= Cursor.y;
				ImGui::Image((void*)(intptr_t)Tex, ImVec2(Size.x, Size.y), ImVec2(0, 1), ImVec2(1, 0));
				SizeOfRenderWindow = { (int)Size.x, (int)Size.y };
			}
			ImGui::End();
			ImGui::PopStyleVar(1);
		}
	}

}


