#include <Editor/PanelRenderSettings.h>
#include <Editor/FontAwesome.h>
#include <Lib/imgui/imgui.h>
#include <Editor/CommonUI.h>
#include <Core/Core.h>

namespace Columbus::Editor
{

	PanelRenderSettings::PanelRenderSettings() : Panel(ICON_FA_COG" Render Settings") {}

	void PanelRenderSettings::DrawInternal()
	{
		if (Render != nullptr)
		{
			if (ImGui::CollapsingHeader("Bloom"))
			{
				const char* Resolutions[] = { "Quad", "Half", "Full" };

				ImGui::PushID("Bloom");
				ImGui::Indent(10.0f);
				ImGui::Checkbox("Enable",        &Render->Bloom.Enabled);
				ImGui::DragFloat("Treshold",     &Render->Bloom.Treshold,  0.001f);
				ImGui::DragFloat("Intensity",    &Render->Bloom.Intensity, 0.001f);
				ImGui::DragFloat("Radius",       &Render->Bloom.Radius,    0.001f);
				ImGui::SliderInt("Iterations",   &Render->Bloom.Iterations, 1, 4);
				ImGui::Combo("Resolution", (int*)&Render->Bloom.Resolution, Resolutions, 3);
				ImGui::Unindent(10.0f);

				Render->Bloom.Iterations = Math::Clamp(Render->Bloom.Iterations, 1, 4);

				ImGui::Separator();
				ImGui::Spacing();
				ImGui::PopID();
			}

			if (ImGui::CollapsingHeader("Vignette"))
			{
				ImGui::PushID("Vignettee");
				ImGui::Indent(10.0f);
				ImGui::Checkbox("Enable",           &Render->Vignette.Enabled);
				ImGui::ColorEdit3("Color",  (float*)&Render->Vignette.Color);
				ImGui::DragFloat2("Center", (float*)&Render->Vignette.Center, 0.01f);
				ImGui::SliderFloat("Intensity",     &Render->Vignette.Intensity,  0.0f, 1.0f);
				ImGui::SliderFloat("Smoothness",    &Render->Vignette.Smoothness, 0.0f, 1.0f);
				ImGui::SliderFloat("Radius",        &Render->Vignette.Radius,     0.0f, 1.0f);
				ImGui::Unindent(10.0f);

				Render->Vignette.Color.Clamp({0.0f}, {1.0f});
				Render->Vignette.Intensity  = Math::Clamp(Render->Vignette.Intensity,  0.0f, 1.0f);
				Render->Vignette.Smoothness = Math::Clamp(Render->Vignette.Smoothness, 0.0f, 1.0f);
				Render->Vignette.Radius     = Math::Clamp(Render->Vignette.Radius,     0.0f, 1.0f);

				ImGui::Separator();
				ImGui::Spacing();
				ImGui::PopID();
			}

			if (ImGui::CollapsingHeader("Auto exposure"))
			{
				ImGui::PushID("Auto exposure");
				ImGui::Indent(10.0f);
				ImGui::Checkbox("Enable",      &Render->AutoExposure.Enabled);
				ImGui::SliderFloat("Min",      &Render->AutoExposure.Min, 0.000f, 5.0f);
				ImGui::SliderFloat("Max",      &Render->AutoExposure.Max, 0.000f, 5.0f);
				ImGui::DragFloat("Speed Up",   &Render->AutoExposure.SpeedUp,   0.1f, 0.001f, FLT_MAX);
				ImGui::DragFloat("Speed Down", &Render->AutoExposure.SpeedDown, 0.1f, 0.001f, FLT_MAX);
				ImGui::Unindent(10.0f);

				Render->AutoExposure.Min        = Math::Clamp(Render->AutoExposure.Min,       0.0f, 5.0f);
				Render->AutoExposure.Max        = Math::Clamp(Render->AutoExposure.Max,       0.0f, 5.0f);
				Render->AutoExposure.SpeedUp    = Math::Clamp(Render->AutoExposure.SpeedUp,   0.1f, FLT_MAX);
				Render->AutoExposure.SpeedDown  = Math::Clamp(Render->AutoExposure.SpeedDown, 0.1f, FLT_MAX);

				ImGui::Separator();
				ImGui::Spacing();
				ImGui::PopID();
			}

			if (ImGui::CollapsingHeader("Color grading"))
			{
				const char* Tonemaps[] = { "Simple", "Filmic", "ACES", "RomBinDaHouse", "Uncharted" };

				ImGui::PushID("Color grading");
				ImGui::Indent(10.0f);
				ImGui::Combo("Tonemapping", (int*)&Render->Tonemapping, Tonemaps, sizeofarray(Tonemaps));
				ImGui::SliderFloat("Gamma", &Render->Gamma, 0.0, 5.0);
				ImGui::SliderFloat("Exposure", &Render->Exposure, 0.0, 5.0);
				ImGui::SliderFloat("Saturation", &Render->Saturation, 0.0, 5.0);
				ImGui::SliderFloat("Hue", &Render->Hue, 0.0, 6.0);
				ImGui::SliderFloat("Temperature", &Render->Temperature, 15.0, 150.0);
				ImGui::ColorEdit3("Lift", (float*)&Render->Lift);
				ImGui::ColorEdit3("Gain", (float*)&Render->Gain);
				ImGui::ColorEdit3("Offset", (float*)&Render->Offset);
				ImGui::Unindent(10.0f);

				ImGui::Separator();
				ImGui::Spacing();
				ImGui::PopID();
			}


			const char* AA[] = { "No", "FXAA", "MSAA 2x", "MSAA 4x", "MSAA 8x", "MSAA 16x", "MSAA 32x" };

			ImGui::Combo("Anti Aliasing", (int*)&Render->Antialiasing, AA, sizeofarray(AA));
		}
	}

	PanelRenderSettings::~PanelRenderSettings() {}

}
