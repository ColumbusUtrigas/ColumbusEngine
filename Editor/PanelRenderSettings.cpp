#include <Editor/PanelRenderSettings.h>
#include <Editor/FontAwesome.h>
#include <Lib/imgui/imgui.h>

namespace Columbus
{

	void EditorPanelRenderSettings::Draw()
	{
		if (Opened)
		{
			if (ImGui::Begin((ICON_FA_COG" " + Name + "##PanelRenderSettings").c_str(), &Opened, ImGuiWindowFlags_NoCollapse))
			{
				if (Render != nullptr)
				{
					if (ImGui::CollapsingHeader("Bloom"))
					{
						const char* Resolutions[] = { "Full", "Half", "Quad" };

						ImGui::Indent(10.0f);
						ImGui::Checkbox("Enable##PanelRenderSettings_Bloom",        &Render->BloomEnable);
						ImGui::DragFloat("Treshold##PanelRenderSettings_Bloom",     &Render->BloomTreshold,  0.001f);
						ImGui::DragFloat("Intensity##PanelRenderSettings_Bloom",    &Render->BloomIntensity, 0.001f);
						ImGui::DragFloat("Radius##PanelRenderSettings_Bloom",       &Render->BloomRadius,    0.001f);
						ImGui::SliderInt("Iterations##PanelRenderSettings_Bloom",   &Render->BloomIterations, 1, 4);
						ImGui::Combo("Resolution##PanelRenderSettings_Bloom", (int*)&Render->BloomResolution, Resolutions, 3);
						ImGui::Unindent(10.0f);

						ImGui::Separator();
						ImGui::Spacing();
					}

					ImGui::SliderFloat("Gamma##PanelRenderSettings", &Render->Gamma, 0.0, 5.0);
					ImGui::SliderFloat("Exposure##PanelRenderSettings", &Render->Exposure, 0.0, 5.0);
				}
			}
			ImGui::End();
		}
	}

	EditorPanelRenderSettings::~EditorPanelRenderSettings() {}

}


