#include <Editor/PanelProfiler.h>
#include <Editor/FontAwesome.h>
#include <Profiling/Profiling.h>
#include <Lib/imgui/imgui.h>
#include <Math/MathUtil.h>
#include <Math/Vector4.h>

namespace Columbus::Editor
{

	void PanelProfiler::SetRedrawTime(float Time)
	{
		RedrawTime = Time;
		T += Time;

		while (T >= (60.0f / GraphCount))
		{
			RedrawTimeCurve[Index % GraphCount] = Time * 1000;
			Index++;
			T -= (60.0f / GraphCount);

			if (Index > GraphCount)
			{
				ZeroIndex++;
				ZeroIndex %= GraphCount;
			}
		}
	}

	void PanelProfiler::Draw()
	{
		if (Opened)
		{
			if (ImGui::Begin(ICON_FA_CHART_BAR" Profiler##PanelProfiler", &Opened, ImGuiWindowFlags_NoCollapse))
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
				ImGui::PlotHistogram(" ", RedrawTimeCurve, GraphCount, ZeroIndex, "Milliseconds for last minute", 0, 50, ImVec2(ImGui::GetWindowContentRegionWidth(), 50));

				double CPUTime       = GetProfileTime(ProfileModule::CPU);
				double AudioTime     = GetProfileTime(ProfileModule::Audio);
				double PhysicsTime   = GetProfileTime(ProfileModule::Physics);
				double ParticlesTime = GetProfileTime(ProfileModule::Particles);
				double CullingTime   = GetProfileTime(ProfileModule::Culling);
				double UpdateTime    = GetProfileTime(ProfileModule::Update);

				ImGui::Dummy(ImVec2(0.0f, 10.0f));
				ImGui::Text("CPU Profiling"); ImGui::SameLine();
				ImGui::Text("(%.3f ms)", CPUTime);
				ImGui::Indent(10.0f);
				ImGui::Text("Audio:     %.3f ms", AudioTime);
				ImGui::Text("Physics:   %.3f ms", PhysicsTime);
				ImGui::Text("Particles: %.3f ms", ParticlesTime);
				ImGui::Text("Culling:   %.3f ms", CullingTime);
				ImGui::Text("Update:    %.3f ms", UpdateTime);
				ImGui::Unindent(10.0f);

				double GPUTime             = GetProfileTimeGPU(ProfileModuleGPU::GPU);
				double OpaqueStage         = GetProfileTimeGPU(ProfileModuleGPU::OpaqueStage);
				double SkyStage            = GetProfileTimeGPU(ProfileModuleGPU::SkyStage);
				double TransparentStage    = GetProfileTimeGPU(ProfileModuleGPU::TransparentStage);
				double BloomStage          = GetProfileTimeGPU(ProfileModuleGPU::BloomStage);
				double FinalStage          = GetProfileTimeGPU(ProfileModuleGPU::FinalStage);

				ImGui::Dummy(ImVec2(0.0f, 10.0f));
				ImGui::Text("GPU Profiling"); ImGui::SameLine();
				ImGui::Text("(%.3f ms)", GPUTime);
				ImGui::Indent(10.0f);
				ImGui::Text("Opaque:      %.3f ms", OpaqueStage);
				ImGui::Text("Sky:         %.3f ms", SkyStage);
				ImGui::Text("Transparent: %.3f ms", TransparentStage);
				ImGui::Text("Bloom:       %.3f ms", BloomStage);
				ImGui::Text("Final:       %.3f ms", FinalStage);
				ImGui::Unindent(10.0f);
			}
			ImGui::End();
		}
	}

	PanelProfiler::~PanelProfiler() {}

}


