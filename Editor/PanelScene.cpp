#include <Editor/PanelScene.h>
#include <Editor/FontAwesome.h>
#include <Editor/CommonUI.h>
#include <Lib/imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
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
					ImGui::Checkbox("Gizmo##PanelScene", &_Gizmo.Enable);

					ImGui::RadioButton("T", (int*)&_Gizmo._Operation, Gizmo::Operation::Translate);
					ImGui::RadioButton("R", (int*)&_Gizmo._Operation, Gizmo::Operation::Rotate);
					ImGui::RadioButton("S", (int*)&_Gizmo._Operation, Gizmo::Operation::Scale);

					ImGui::EndMenuBar();
				}

				Selected = ImGui::IsWindowFocused();

				Position = iVector2((int)ImGui::GetCursorScreenPos().x, (int)ImGui::GetCursorScreenPos().y);

				if (FramebufferTexture != nullptr)
				{
					auto Tex = ((TextureOpenGL*)FramebufferTexture)->GetID();
					auto Cursor = ImGui::GetCursorPos();
					auto Size = ImGui::GetWindowSize();
					Size.y -= Cursor.y;
					ImGui::Image((void*)(intptr_t)Tex, ImVec2(Size.x, Size.y), ImVec2(0, 1), ImVec2(1, 0));
					Hover = ImGui::IsItemHovered();
					SizeOfRenderWindow = { (int)Size.x, (int)Size.y };

					if (_PickedObject != nullptr)
					{
						Vector4 rect(Position.X, Position.Y, Size.x, Size.y);
						_Gizmo.SetCamera(Render.GetMainCamera());
						_Gizmo.Draw(_PickedObject->transform, rect);
					}
				}
			}
			ImGui::End();
			ImGui::PopStyleVar(1);
		}
	}

}


