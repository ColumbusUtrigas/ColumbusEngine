#include <Editor/Inspector/ComponentEditor.h>
#include <Scene/ComponentParticleSystem.h>
#include <imgui/imgui.h>

namespace Columbus
{

	class ComponentEditorParticleSystem : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentParticleSystem, ComponentEditorParticleSystem);
	public:
		void OnInspectorGUI() final override
		{
			auto Co = static_cast<ComponentParticleSystem*>(Target);
			if (Co != nullptr)
			{
				const char* BlendModes[] = { "Default", "Add", "Subtract", "Multiply" };
				const char* BillboardModes[] = { "None", "Vertical", "Horizontal", "Face to camera" };
				const char* TransformationModes[] = { "World", "Local" };
				const char* SortModes[] = { "None", "Young first", "Old first", "Nearest first" };

#define NAME(a)    a"##PanelInspector_ParticleSystem"

				auto Emitter = &Co->GetEmitter();

				ImGui::Indent(10.0f);

				ImGui::DragInt  (NAME("Max Particles"), (int*)&Emitter->MaxParticles, 1, 0, 1024);
				ImGui::DragFloat(NAME("Emit Rate"), &Emitter->EmitRate, 0.1f, 0.0f, FLT_MAX);
				ImGui::Checkbox (NAME("Emit"), &Emitter->Emit);
				ImGui::Checkbox (NAME("Visible"), &Emitter->Visible);

				ImGui::Combo(NAME("Blend mode"), (int*)&Emitter->Blend, BlendModes, 4);
				ImGui::Combo(NAME("Billboard mode"), (int*)&Emitter->Billboard, BillboardModes, 4);
				ImGui::Combo(NAME("Transformation mode"), (int*)&Emitter->Transformation, TransformationModes, 2);
				ImGui::Combo(NAME("Sort mode"), (int*)&Emitter->Sort, SortModes, 4);

				ImGui::Separator();

				if (ImGui::CollapsingHeader(NAME("Lifetime")))
				{
					ImGui::Indent(10.0f);
					ImGui::DragFloatRange2(NAME("Lifetime##Lifetime"), &Emitter->ModuleLifetime.Min, &Emitter->ModuleLifetime.Max, 0.1f);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Location")))
				{
					const char* Shapes[] = { "Point", "Box", "Circle", "Sphere" };

					ImGui::Indent(10.0f);
					ImGui::DragFloat3(NAME("Size##Location"), (float*)&Emitter->ModuleLocation.Size, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat(NAME("Radius##Location"), &Emitter->ModuleLocation.Radius, 0.1f, 0.0f, FLT_MAX);
					ImGui::Combo(NAME("Shape##Location"), (int*)&Emitter->ModuleLocation.Shape, Shapes, 4);
					ImGui::Checkbox(NAME("Emit from shell##Location"), &Emitter->ModuleLocation.EmitFromShell);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Velocity")))
				{
					ImGui::Indent(10.0f);
					ImGui::DragFloat3(NAME("Min##Velocity"), (float*)&Emitter->ModuleVelocity.Min, 0.1f);
					ImGui::DragFloat3(NAME("Max##Velocity"), (float*)&Emitter->ModuleVelocity.Max, 0.1f);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Rotation")))
				{
					ImGui::Indent(10.0f);
					ImGui::DragFloatRange2("Initial##PanelInspector_ParticleSystem_Rotation", &Emitter->ModuleRotation.Min, &Emitter->ModuleRotation.Max, 0.1f);
					ImGui::DragFloatRange2("Velocity##PanelInspector_ParticleSystem_Rotation", &Emitter->ModuleRotation.MinVelocity, &Emitter->ModuleRotation.MaxVelocity, 0.1f);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Color")))
				{
					const char* Modes[] = { "Initial", "Over life" };

					ImGui::Indent(10.0f);

					ImGui::Combo(NAME("Mode##Color"), (int*)&Emitter->ModuleColor.Mode, Modes, 2);

					if (Emitter->ModuleColor.Mode == ParticleModuleColor::UpdateMode::Initial)
					{
						ImGui::ColorEdit4(NAME("Min##Color"), (float*)&Emitter->ModuleColor.Min);
						ImGui::ColorEdit4(NAME("Max##Color"), (float*)&Emitter->ModuleColor.Max);
					}

					if (Emitter->ModuleColor.Mode == ParticleModuleColor::UpdateMode::OverLife)
					{
						bool Editing = false;
						size_t Counter = 1;
						for (auto& Node : Emitter->ModuleColor.Curve.Points)
						{
							auto str = std::to_string(Counter);

							float Min = Counter == 1 ? 0.0f : Emitter->ModuleColor.Curve.Points[Counter - 2].Key;
							float Max = Counter == Emitter->ModuleColor.Curve.Points.size() ? 1.0f : Emitter->ModuleColor.Curve.Points[Counter].Key;

							if (ImGui::CollapsingHeader((str + NAME("##ColorOverLife_Point")).c_str()))
							{
								auto p = NAME("Value##ColorOverLife") + str;
								auto c = NAME("Color##ColorOverLife") + str;
								auto r = NAME("Remove##ColorOverLife") + str;

								ImGui::Indent(10.0f);
								ImGui::SetNextItemWidth(50.0f);

								ImGui::DragFloat(p.c_str(), &Node.Key, 0.05f, Min, Max);
								if (ImGui::IsItemActive())
									Editing = true;

								ImGui::ColorEdit4(c.c_str(), (float*)&Node.Value);

								if (ImGui::Button(r.c_str())) Emitter->ModuleColor.Curve.RemovePoint(Counter - 1);
								ImGui::Unindent(10.0f);
							}

							Counter++;
						}

						if (!Editing)
						{
							Emitter->ModuleColor.Curve.Sort();
						}

						if (ImGui::Button(NAME("Add##ColorOverLife")))
						{
							Emitter->ModuleColor.Curve.AddPoint(Vector4(1), 1.0f);
						}
					}

					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Size")))
				{
					const char* Modes[] = { "Initial", "Over life" };

					ImGui::Indent(10.0f);

					ImGui::Combo(NAME("Mode##Size"), (int*)&Emitter->ModuleSize.Mode, Modes, 2);

					if (Emitter->ModuleSize.Mode == ParticleModuleSize::UpdateMode::Initial)
					{
						ImGui::DragFloat3(NAME("Min##Size"), (float*)&Emitter->ModuleSize.Min);
						ImGui::DragFloat3(NAME("Max##Size"), (float*)&Emitter->ModuleSize.Max);
					}

					if (Emitter->ModuleSize.Mode == ParticleModuleSize::UpdateMode::OverLife)
					{
						bool Editing = false;
						size_t Counter = 1;
						for (auto& Node : Emitter->ModuleSize.Curve.Points)
						{
							auto str = std::to_string(Counter);

							float Min = Counter == 1 ? 0.0f : Emitter->ModuleSize.Curve.Points[Counter - 2].Key;
							float Max = Counter == Emitter->ModuleSize.Curve.Points.size() ? 1.0f : Emitter->ModuleSize.Curve.Points[Counter].Key;

							if (ImGui::CollapsingHeader((str + NAME("##SizeOverLife_Point")).c_str()))
							{
								auto p = NAME("Value##SizeOverLife") + str;
								auto c = NAME("Size##SizeOverLife") + str;
								auto r = NAME("Remove##SizeOverLife") + str;

								ImGui::Indent(10.0f);
								ImGui::SetNextItemWidth(50.0f);

								ImGui::DragFloat(p.c_str(), &Node.Key, 0.05f, Min, Max);
								if (ImGui::IsItemActive())
									Editing = true;

								ImGui::DragFloat3(c.c_str(), (float*)&Node.Value, 0.1f, 0.0f, FLT_MAX);
								if (ImGui::Button(r.c_str())) Emitter->ModuleSize.Curve.RemovePoint(Counter - 1);
								ImGui::Unindent(10.0f);
							}

							Counter++;
						}

						if (!Editing)
						{
							Emitter->ModuleSize.Curve.Sort();
						}

						if (ImGui::Button(NAME("Add##SizeOverLife")))
						{
							Emitter->ModuleSize.Curve.AddPoint(Vector3(1), 1.0f);
						}
					}

					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Noise")))
				{
					ImGui::Indent(10.0f);

					ImGui::DragFloat(NAME("Strength##Noise"), &Emitter->ModuleNoise.Strength, 0.1f, 0.0f, FLT_MAX);
					ImGui::SliderInt(NAME("Octaves##Noise"), &Emitter->ModuleNoise.Octaves, 1, 8);
					ImGui::DragFloat(NAME("Lacunarity##Noise"), &Emitter->ModuleNoise.Lacunarity, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat(NAME("Persistence##Noise"), &Emitter->ModuleNoise.Persistence, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat(NAME("Frequency##Noise"), &Emitter->ModuleNoise.Frequency, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat(NAME("Amplitude##Noise"), &Emitter->ModuleNoise.Amplitude, 0.1f, 0.0f, FLT_MAX);

					ImGui::Unindent(10.0f);
				}

				if (ImGui::CollapsingHeader(NAME("SubUV")))
				{
					const char* Modes[] = { "Linear", "Random" };

					ImGui::Indent(10.0f);

					ImGui::Combo(NAME("Mode##SubUV"), (int*)&Emitter->ModuleSubUV.Mode, Modes, 2);
					ImGui::DragInt(NAME("Horizontal##SubUV"), &Emitter->ModuleSubUV.Horizontal, 1, 0, INT_MAX);
					ImGui::DragInt(NAME("Vertical##SubUV"), &Emitter->ModuleSubUV.Vertical, 1, 0, INT_MAX);
					ImGui::DragFloat(NAME("Cycles##SubUV"), &Emitter->ModuleSubUV.Cycles, 0.1f, 0.0f, FLT_MAX);

					ImGui::Unindent(10.0f);
				}

				ImGui::Unindent(10.0f);
				ImGui::Separator();

#undef NAME
			}
		}
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentParticleSystem, ComponentEditorParticleSystem);

}
