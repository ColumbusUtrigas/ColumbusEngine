#include <Editor/PanelRenderSettings.h>
#include <Editor/FontAwesome.h>
#include <Lib/imgui/imgui.h>

namespace Columbus
{

	void EditorPanelRenderSettings::Draw()
	{
		if (Opened)
		{
			if (ImGui::Begin(ICON_FA_COG" Render Settings##PanelRenderSettings", &Opened, ImGuiWindowFlags_NoCollapse))
			{
				if (Render != nullptr)
				{
					if (ImGui::CollapsingHeader("Bloom"))
					{
						const char* Resolutions[] = { "Quad", "Half", "Full" };

						ImGui::Indent(10.0f);
						ImGui::Checkbox("Enable##PanelRenderSettings_Bloom",        &Render->Bloom.Enabled);
						ImGui::DragFloat("Treshold##PanelRenderSettings_Bloom",     &Render->Bloom.Treshold,  0.001f);
						ImGui::DragFloat("Intensity##PanelRenderSettings_Bloom",    &Render->Bloom.Intensity, 0.001f);
						ImGui::DragFloat("Radius##PanelRenderSettings_Bloom",       &Render->Bloom.Radius,    0.001f);
						ImGui::SliderInt("Iterations##PanelRenderSettings_Bloom",   &Render->Bloom.Iterations, 1, 4);
						ImGui::Combo("Resolution##PanelRenderSettings_Bloom", (int*)&Render->Bloom.Resolution, Resolutions, 3);
						ImGui::Unindent(10.0f);

						Render->Bloom.Iterations = Math::Clamp(Render->Bloom.Iterations, 1, 4);

						ImGui::Separator();
						ImGui::Spacing();
					}

					if (ImGui::CollapsingHeader("Vignette"))
					{
						ImGui::Indent(10.0f);
						ImGui::Checkbox("Enable##PanelRenderSettings_Vignette",           &Render->Vignette.Enabled);
						ImGui::ColorEdit3("Color##PanelRenderSettings_Vignette",  (float*)&Render->Vignette.Color);
						ImGui::DragFloat2("Center##PanelRenderSettings_Vignette", (float*)&Render->Vignette.Center, 0.01f);
						ImGui::SliderFloat("Intensity##PanelRenderSettings_Vignette",     &Render->Vignette.Intensity,  0.0f, 1.0f);
						ImGui::SliderFloat("Smoothness##PanelRenderSettings_Vignette",    &Render->Vignette.Smoothness, 0.0f, 1.0f);
						ImGui::SliderFloat("Radius##PanelRenderSettings_Vignette",        &Render->Vignette.Radius,     0.0f, 1.0f);
						ImGui::Unindent(10.0f);

						Render->Vignette.Color.Clamp({0.0f}, {1.0f});
						Render->Vignette.Intensity  = Math::Clamp(Render->Vignette.Intensity,  0.0f, 1.0f);
						Render->Vignette.Smoothness = Math::Clamp(Render->Vignette.Smoothness, 0.0f, 1.0f);
						Render->Vignette.Radius     = Math::Clamp(Render->Vignette.Radius,     0.0f, 1.0f);

						ImGui::Separator();
						ImGui::Spacing();
					}

					if (ImGui::CollapsingHeader("Auto exposure"))
					{
						ImGui::Indent(10.0f);
						ImGui::Checkbox("Enable##PanelRenderSettings_AutoExposure",      &Render->AutoExposure.Enabled);
						ImGui::SliderFloat("Min##PanelRenderSettings_AutoExposure",      &Render->AutoExposure.Min, 0.000f, 5.0f);
						ImGui::SliderFloat("Max##PanelRenderSettings_AutoExposure",      &Render->AutoExposure.Max, 0.000f, 5.0f);
						ImGui::DragFloat("Speed Up##PanelRenderSettings_AutoExposure",   &Render->AutoExposure.SpeedUp,   0.1f, 0.001f, FLT_MAX);
						ImGui::DragFloat("Speed Down##PanelRenderSettings_AutoExposure", &Render->AutoExposure.SpeedDown, 0.1f, 0.001f, FLT_MAX);
						ImGui::Unindent(10.0f);

						Render->AutoExposure.Min        = Math::Clamp(Render->AutoExposure.Min,       0.0f, 5.0f);
						Render->AutoExposure.Max        = Math::Clamp(Render->AutoExposure.Max,       0.0f, 5.0f);
						Render->AutoExposure.SpeedUp    = Math::Clamp(Render->AutoExposure.SpeedUp,   0.1f, FLT_MAX);
						Render->AutoExposure.SpeedDown  = Math::Clamp(Render->AutoExposure.SpeedDown, 0.1f, FLT_MAX);

						ImGui::Separator();
						ImGui::Spacing();
					}


					const char* Tonemaps[] = { "Simple", "Filmic", "ACES", "RomBinDaHouse", "Uncharted" };
					const char* AA[] = { "No", "FXAA", "MSAA 2x", "MSAA 4x", "MSAA 8x", "MSAA 16x", "MSAA 32x" };

					ImGui::Combo("Tonemapping##", (int*)&Render->Tonemapping, Tonemaps, 5);
					ImGui::SliderFloat("Gamma##PanelRenderSettings", &Render->Gamma, 0.0, 5.0);
					ImGui::SliderFloat("Exposure##PanelRenderSettings", &Render->Exposure, 0.0, 5.0);
					ImGui::Combo("Anti Aliasing##PanelRenderSettings", (int*)&Render->Antialiasing, AA, 7);
				}
			}
			ImGui::End();
		}
	}

	EditorPanelRenderSettings::~EditorPanelRenderSettings() {}

}


