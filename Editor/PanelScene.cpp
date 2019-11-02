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
					ImGui::Checkbox("Grid##PanelScene", &Render.DrawGrid);
					ImGui::EndMenuBar();
				}

				Selected = ImGui::IsWindowFocused();

				Position = iVector2((int)ImGui::GetCursorScreenPos().x, (int)ImGui::GetCursorScreenPos().y);

				auto Tex = ((TextureOpenGL*)FramebufferTexture)->GetID();
				auto Cursor = ImGui::GetCursorPos();
				auto Size = ImGui::GetWindowSize();
				Size.y -= Cursor.y;
				ImGui::Image((void*)(intptr_t)Tex, ImVec2(Size.x, Size.y), ImVec2(0, 1), ImVec2(1, 0));
				Hover = ImGui::IsItemHovered();
				SizeOfRenderWindow = { (int)Size.x, (int)Size.y };

				Render.EnableMousePicking = false;
				auto mousePos = ImGui::GetMousePos();
				if (mousePos.x > Position.X && mousePos.x < Position.X + Size.x &&
				    mousePos.y > Position.Y && mousePos.y < Position.Y + Size.y)
				{
					Vector2 pickPos;
					pickPos.X = mousePos.x - Position.X;
					pickPos.Y = mousePos.y - Position.Y;
					pickPos = pickPos / Vector2(SizeOfRenderWindow) * 2 - 1;

					Render.EnableMousePicking = true;
					Render.MousePickingPosition = pickPos;
				}
			}
			ImGui::End();
			ImGui::PopStyleVar(1);
		}
	}

}


