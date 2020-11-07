#include <Editor/Inspector/ComponentEditor.h>
#include <Editor/CommonUI.h>
#include <Editor/Icons.h>
#include <Scene/ComponentParticleSystem.h>
#include <imgui/imgui.h>
#include <functional>

namespace Columbus::Editor
{

	class ComponentEditorParticleSystem : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentParticleSystem, ComponentEditorParticleSystem);
	public:
		void OnInspectorGUI() final override;
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentParticleSystem, ComponentEditorParticleSystem);

	static void DrawModuleEditor(std::function<void(ParticleEmitterCPU&)> func, const char* name, ParticleEmitterCPU& Emitter)
	{
		if (ImGui::CollapsingHeader(name))
		{
			ImGui::Indent(10);
			ImGui::PushID(name);
			func(Emitter);
			ImGui::PopID();
			ImGui::Unindent(10);
			ImGui::Separator();
		}
	}

	static void DrawLifetime(ParticleEmitterCPU& Emitter)
	{
		ImGui::DragFloatRange2("Lifetime", &Emitter.ModuleLifetime.Min, &Emitter.ModuleLifetime.Max, 0.1f);
	}

	static void DrawLocation(ParticleEmitterCPU& Emitter)
	{
		const char* Shapes[] = { "Point", "Box", "Circle", "Sphere" };

		ImGui::DragFloat3("Size", (float*)&Emitter.ModuleLocation.Size, 0.1f, 0.0f, FLT_MAX);
		ImGui::DragFloat("Radius", &Emitter.ModuleLocation.Radius, 0.1f, 0.0f, FLT_MAX);
		ImGui::Combo("Shape", (int*)&Emitter.ModuleLocation.Shape, Shapes, sizeof(Shapes) / sizeof(Shapes[0]));
		ImGui::Checkbox("Emit from shell", &Emitter.ModuleLocation.EmitFromShell);
	}

	static void DrawVelocity(ParticleEmitterCPU& Emitter)
	{
		ImGui::DragFloat3("Min", (float*)&Emitter.ModuleVelocity.Min, 0.1f);
		ImGui::DragFloat3("Max", (float*)&Emitter.ModuleVelocity.Max, 0.1f);
	}

	static void DrawRotation(ParticleEmitterCPU& Emitter)
	{
		ImGui::DragFloatRange2("Initial", &Emitter.ModuleRotation.Min, &Emitter.ModuleRotation.Max, 0.1f);
		ImGui::DragFloatRange2("Velocity", &Emitter.ModuleRotation.MinVelocity, &Emitter.ModuleRotation.MaxVelocity, 0.1f);
	}

	static void DrawColor(ParticleEmitterCPU& Emitter)
	{
		const char* Modes[] = { "Initial", "Over life" };

		ImGui::Combo("Mode", (int*)&Emitter.ModuleColor.Mode, Modes, 2);

		if (Emitter.ModuleColor.Mode == ParticleModuleColor::UpdateMode::Initial)
		{
			ImGui::ColorEdit4("Min", (float*)&Emitter.ModuleColor.Min);
			ImGui::ColorEdit4("Max", (float*)&Emitter.ModuleColor.Max);
		}

		if (Emitter.ModuleColor.Mode == ParticleModuleColor::UpdateMode::OverLife)
		{
			bool Editing = false;
			char buf[64];

			if (ImGui::Button(ADD_ICON))
			{
				Emitter.ModuleColor.Curve.AddPoint(Vector4(1), 1.0f);
			}
			ShowTooltipDelayed(CommonUISettings.TooltipDelay, "Add new color point");

			for (size_t i = 0; i < Emitter.ModuleColor.Curve.Points.size(); i++)
			{
				auto& Node = Emitter.ModuleColor.Curve.Points[i];

				float Min = i == 0 ? 0.0f : Emitter.ModuleColor.Curve.Points[i - 1].Key;
				float Max = i == Emitter.ModuleColor.Curve.Points.size() - 1 ? 1.0f : Emitter.ModuleColor.Curve.Points[i + 1].Key;

				ImGui::PushID(i);

				ImGui::AlignTextToFramePadding();
				bool Open = ImGui::TreeNodeSized(itoa(i + 1, buf, 10), {}, ImGuiTreeNodeFlags_SpanAvailWidth);

				ImGui::SameLine();
				if (ImGui::Button(DELETE_ICON))
				{
					Emitter.ModuleColor.Curve.RemovePoint(i);
					Open = false;
				}

				if (Open)
				{
					ImGui::TreePush();
					ImGui::SetNextItemWidth(50);

					ImGui::DragFloat("Value", &Node.Key, 0.05f, Min, Max);
					if (ImGui::IsItemActive())
						Editing = true;

					ImGui::ColorEdit4("Color", (float*)&Node.Value);

					ImGui::TreePop();
				}
				ImGui::PopID();
			}

			if (!Editing)
			{
				Emitter.ModuleColor.Curve.Sort();
			}
		}
	}

	static void DrawSize(ParticleEmitterCPU& Emitter)
	{
		const char* Modes[] = { "Initial", "Over life" };

		ImGui::Combo("Mode", (int*)&Emitter.ModuleSize.Mode, Modes, 2);

		if (Emitter.ModuleSize.Mode == ParticleModuleSize::UpdateMode::Initial)
		{
			ImGui::DragFloat3("Min", (float*)&Emitter.ModuleSize.Min);
			ImGui::DragFloat3("Max", (float*)&Emitter.ModuleSize.Max);
		}
		
		if (Emitter.ModuleSize.Mode == ParticleModuleSize::UpdateMode::OverLife)
		{
			bool Editing = false;
			char buf[64];

			if (ImGui::Button(ADD_ICON))
			{
				Emitter.ModuleSize.Curve.AddPoint(Vector3(1), 1.0f);
			}
			ShowTooltipDelayed(CommonUISettings.TooltipDelay, "Add new size point");

			for (size_t i = 0; i < Emitter.ModuleSize.Curve.Points.size(); i++)
			{
				auto& Node = Emitter.ModuleSize.Curve.Points[i];

				float Min = i == 0 ? 0.0f : Emitter.ModuleSize.Curve.Points[i - 1].Key;
				float Max = i == Emitter.ModuleSize.Curve.Points.size()-1 ? 1.0f : Emitter.ModuleSize.Curve.Points[i + 1].Key;

				ImGui::PushID(i);

				ImGui::AlignTextToFramePadding();
				bool Open = ImGui::TreeNodeSized(itoa(i + 1, buf, 10), {}, ImGuiTreeNodeFlags_SpanAvailWidth);

				ImGui::SameLine();
				if (ImGui::Button(DELETE_ICON))
				{
					Emitter.ModuleSize.Curve.RemovePoint(i);
					Open = false;
				}

				if (Open)
				{
					ImGui::TreePush();
					ImGui::SetNextItemWidth(50.0f);

					ImGui::DragFloat("Value", &Node.Key, 0.05f, Min, Max);
					if (ImGui::IsItemActive())
						Editing = true;

					ImGui::DragFloat3("Size", (float*)&Node.Value, 0.1f, 0.0f, FLT_MAX);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}

			if (!Editing)
			{
				Emitter.ModuleSize.Curve.Sort();
			}
		}
	}

	static void DrawNoise(ParticleEmitterCPU& Emitter)
	{
		ImGui::DragFloat("Strength",    &Emitter.ModuleNoise.Strength,    0.1f, 0.0f, FLT_MAX);
		ImGui::SliderInt("Octaves",     &Emitter.ModuleNoise.Octaves,     1, 8);
		ImGui::DragFloat("Lacunarity",  &Emitter.ModuleNoise.Lacunarity,  0.1f, 0.0f, FLT_MAX);
		ImGui::DragFloat("Persistence", &Emitter.ModuleNoise.Persistence, 0.1f, 0.0f, FLT_MAX);
		ImGui::DragFloat("Frequency",   &Emitter.ModuleNoise.Frequency,   0.1f, 0.0f, FLT_MAX);
		ImGui::DragFloat("Amplitude",   &Emitter.ModuleNoise.Amplitude,   0.1f, 0.0f, FLT_MAX);
	}

	static void DrawSubUV(ParticleEmitterCPU& Emitter)
	{
		const char* Modes[] = { "Linear", "Random" };

		ImGui::Combo("Mode", (int*)&Emitter.ModuleSubUV.Mode, Modes, 2);
		ImGui::DragInt("Horizontal", &Emitter.ModuleSubUV.Horizontal, 1, 0, INT_MAX);
		ImGui::DragInt("Vertical", &Emitter.ModuleSubUV.Vertical, 1, 0, INT_MAX);
		ImGui::DragFloat("Cycles", &Emitter.ModuleSubUV.Cycles, 0.1f, 0.0f, FLT_MAX);
	}

	void ComponentEditorParticleSystem::OnInspectorGUI()
	{
		auto Co = static_cast<ComponentParticleSystem*>(Target);
		if (Co != nullptr)
		{
			const char* BlendModes[] = { "Default", "Add", "Subtract", "Multiply" };
			const char* BillboardModes[] = { "None", "Vertical", "Horizontal", "Face to camera" };
			const char* TransformationModes[] = { "World", "Local" };
			const char* SortModes[] = { "None", "Young first", "Old first", "Nearest first" };

			auto& Emitter = Co->GetEmitter();

			ImGui::DragInt("Max Particles", (int*)&Emitter.MaxParticles, 1, 0, 1024);
			ImGui::DragFloat("Emit Rate", &Emitter.EmitRate, 0.1f, 0.0f, FLT_MAX);
			ImGui::Checkbox("Emit", &Emitter.Emit);
			ImGui::Checkbox("Visible", &Emitter.Visible);
			ImGui::Checkbox("Light", &Emitter.Light);

			ImGui::Combo("Blend mode", (int*)&Emitter.Blend, BlendModes, 4);
			ImGui::Combo("Billboard mode", (int*)&Emitter.Billboard, BillboardModes, 4);
			ImGui::Combo("Transformation mode", (int*)&Emitter.Transformation, TransformationModes, 2);
			ImGui::Combo("Sort mode", (int*)&Emitter.Sort, SortModes, 4);

			ImGui::Separator();

			DrawModuleEditor(DrawLifetime, "Lifetime", Emitter);
			DrawModuleEditor(DrawLocation, "Location", Emitter);
			DrawModuleEditor(DrawVelocity, "Velocity", Emitter);
			DrawModuleEditor(DrawRotation, "Rotation", Emitter);
			DrawModuleEditor(DrawColor, "Color", Emitter);
			DrawModuleEditor(DrawSize, "Size", Emitter);
			DrawModuleEditor(DrawNoise, "Noise", Emitter);
			DrawModuleEditor(DrawSubUV, "SubUV", Emitter);
		}
	}

}
