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
						ImGui::Checkbox("Enable##PanelRenderSettings_Bloom",        &Render->BloomEnable);
						ImGui::DragFloat("Treshold##PanelRenderSettings_Bloom",     &Render->BloomTreshold,  0.001f);
						ImGui::DragFloat("Intensity##PanelRenderSettings_Bloom",    &Render->BloomIntensity, 0.001f);
						ImGui::DragFloat("Radius##PanelRenderSettings_Bloom",       &Render->BloomRadius,    0.001f);
						ImGui::SliderInt("Iterations##PanelRenderSettings_Bloom",   &Render->BloomIterations, 1, 4);
						ImGui::Combo("Resolution##PanelRenderSettings_Bloom", (int*)&Render->BloomResolution, Resolutions, 3);
						ImGui::Unindent(10.0f);

						Render->BloomIterations = Math::Clamp(Render->BloomIterations, 1, 4);

						ImGui::Separator();
						ImGui::Spacing();
					}

					if (ImGui::CollapsingHeader("Vignette"))
					{
						ImGui::Indent(10.0f);
						ImGui::Checkbox("Enable##PanelRenderSettings_Vignette",           &Render->VignetteEnable);
						ImGui::ColorEdit3("Color##PanelRenderSettings_Vignette",  (float*)&Render->VignetteColor);
						ImGui::DragFloat2("Center##PanelRenderSettings_Vignette", (float*)&Render->VignetteCenter, 0.01f);
						ImGui::SliderFloat("Intensity##PanelRenderSettings_Vignette",     &Render->VignetteIntensity,  0.0f, 1.0f);
						ImGui::SliderFloat("Smoothness##PanelRenderSettings_Vignette",    &Render->VignetteSmoothness, 0.0f, 1.0f);
						ImGui::SliderFloat("Radius##PanelRenderSettings_Vignette",        &Render->VignetteRadius,     0.0f, 1.0f);
						ImGui::Unindent(10.0f);

						Render->VignetteColor.Clamp({0.0f}, {1.0f});
						Render->VignetteIntensity  = Math::Clamp(Render->VignetteIntensity,  {0.0f}, {1.0f});
						Render->VignetteSmoothness = Math::Clamp(Render->VignetteSmoothness, {0.0f}, {1.0f});
						Render->VignetteRadius     = Math::Clamp(Render->VignetteRadius,     {0.0f}, {1.0f});

						ImGui::Separator();
						ImGui::Spacing();
					}

					if (ImGui::CollapsingHeader("Eye adaptation"))
					{
						ImGui::Indent(10.0f);
						ImGui::Checkbox("Enable##PanelRenderSettings_EyeAdaptation",      &Render->EyeAdaptationEnable);
						ImGui::SliderFloat("Min##PanelRenderSettings_EyeAdaptation",      &Render->EyeAdaptationMin, 0.000f, 5.0f);
						ImGui::SliderFloat("Max##PanelRenderSettings_EyeAdaptation",      &Render->EyeAdaptationMax, 0.000f, 5.0f);
						ImGui::DragFloat("Speed Up##PanelRenderSettings_EyeAdaptation",   &Render->EyeAdaptationSpeedUp,   0.1f, 0.001f, FLT_MAX);
						ImGui::DragFloat("Speed Down##PanelRenderSettings_EyeAdaptation", &Render->EyeAdaptationSpeedDown, 0.1f, 0.001f, FLT_MAX);
						ImGui::Unindent(10.0f);

						Render->VignetteColor.Clamp({0.0f}, {1.0f});
						Render->VignetteIntensity  = Math::Clamp(Render->VignetteIntensity,  {0.0f}, {1.0f});
						Render->VignetteSmoothness = Math::Clamp(Render->VignetteSmoothness, {0.0f}, {1.0f});
						Render->VignetteRadius     = Math::Clamp(Render->VignetteRadius,     {0.0f}, {1.0f});

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


