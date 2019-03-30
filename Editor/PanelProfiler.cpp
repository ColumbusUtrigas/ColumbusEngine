#include <Editor/PanelProfiler.h>
#include <Lib/imgui/imgui.h>
#include <Math/MathUtil.h>
#include <Math/Vector4.h>

namespace Columbus
{

	void EditorPanelProfiler::Draw()
	{
		if (Opened)
		{
			if (ImGui::Begin(Name.c_str(), &Opened))
			{
				Vector3 Color(0, 1, 0);
				float Framerate = RedrawTime != 0.0f ? (1.0 / RedrawTime) : 0.0f;

				if (Framerate >= 0.0f && Framerate <= 60.0f)
				{
					Color = Math::Mix(Vector3(1, 0, 0), Vector3(0, 1, 0), Framerate / 60.0f);
				}

				ImVec4 TextColor = ImVec4(Color.X, Color.Y, Color.Z, 1.0f);
				ImGui::TextColored(TextColor, "Redraw time: %.1f ms",  RedrawTime * 1000);
				ImGui::TextColored(TextColor, "Framerate:   %.1f FPS", Framerate);
			}
			ImGui::End();
		}
	}

	EditorPanelProfiler::~EditorPanelProfiler() {}

}


