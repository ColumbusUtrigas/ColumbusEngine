#include <Editor/PanelRenderSettings.h>
#include <Lib/imgui/imgui.h>

namespace Columbus
{

	void EditorPanelRenderSettings::Draw()
	{
		if (Opened)
		{
			if (ImGui::Begin(Name.c_str(), &Opened, ImGuiWindowFlags_NoCollapse))
			{
				if (Render != nullptr)
				{
					if (ImGui::CollapsingHeader("Bloom"))
					{
						const char* Resolutions[] = { "Full", "Half", "Quad" };

						ImGui::Indent(10.0f);
						ImGui::Checkbox("Enable",        &Render->BloomEnable);
						ImGui::DragFloat("Treshold",     &Render->BloomTreshold,  0.001f);
						ImGui::DragFloat("Intensity",    &Render->BloomIntensity, 0.001f);
						ImGui::DragFloat("Radius",       &Render->BloomRadius,    0.001f);
						ImGui::SliderInt("Iterations",   &Render->BloomIterations, 1, 4);
						ImGui::Combo("Resolution", (int*)&Render->BloomResolution, Resolutions, 3);
						ImGui::Unindent(10.0f);

						ImGui::Separator();
						ImGui::Spacing();
					}

					ImGui::SliderFloat("Gamma", &Render->Gamma, 0.0, 5.0);
					ImGui::SliderFloat("Exposure", &Render->Exposure, 0.0, 5.0);
				}
			}
			ImGui::End();
		}
	}

	EditorPanelRenderSettings::~EditorPanelRenderSettings() {}

}


