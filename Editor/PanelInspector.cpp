#include <Editor/PanelInspector.h>
#include <Editor/FontAwesome.h>
#include <Scene/ComponentLight.h>
#include <Scene/ComponentParticleSystem.h>
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>
#include <climits>

namespace Columbus
{

	static void DrawTransformEditor(GameObject* GO);
	static void DrawMaterialEditor(GameObject* GO);
	static void DrawComponentsEditor(GameObject* GO);
	static void DrawComponentEditor(ComponentLight* Co);
	static void DrawComponentEditor(ComponentParticleSystem* Co);

	void EditorPanelInspector::Draw()
	{
		if (Opened)
		{
			if (ImGui::Begin((ICON_FA_INFO" " + Name + "##PanelInspector").c_str(), &Opened, ImGuiWindowFlags_NoCollapse))
			{
				if (Inspectable != nullptr)
				{
					ImGui::InputText("##PanelInspector_Name", &Inspectable->Name);
					ImGui::Separator();

					if (ImGui::CollapsingHeader(ICON_FA_ARROWS" Transform##PanelInspector_Transform", ImGuiTreeNodeFlags_DefaultOpen))
					{
						DrawTransformEditor(Inspectable);
						ImGui::Separator();
					}

					if (ImGui::CollapsingHeader(ICON_FA_CIRCLE" Material##PanelInspector_Material"))
					{
						DrawMaterialEditor(Inspectable);
						ImGui::Separator();
					}

					DrawComponentsEditor(Inspectable);

					ImGui::Separator();
					if (ImGui::Button("Add##PanelInspector_AddComponent"))
					{
						
					}
				}
			}
			ImGui::End();
		}
	}

	EditorPanelInspector::~EditorPanelInspector() {}

	void DrawTransformEditor(GameObject* GO)
	{
		ImGui::Indent(10.0f);
		ImGui::DragFloat3("Position", (float*)&GO->transform.Position, 0.1f);
		ImGui::DragFloat3("Rotation", (float*)&GO->transform.Rotation, 0.1f);
		ImGui::DragFloat3("Scale",    (float*)&GO->transform.Scale,    0.1f);
		ImGui::Unindent(10.0f);
	}

	void DrawMaterialEditor(GameObject* GO)
	{
		const char* CullItems[] = { "No", "Front", "Back", "Front and back"};
		const char* DepthItems[] = { "Less", "Greater", "LEqual", "GEqual", "Equal", "Not equal", "Never", "Always" };

		ImGui::Indent(10.0f);

		ImGui::Combo("Culling",    (int*)&GO->GetMaterial().Culling,      CullItems,  4);
		ImGui::Combo("Depth test", (int*)&GO->GetMaterial().DepthTesting, DepthItems, 8);
		ImGui::Spacing();

		ImGui::Checkbox("Depth writing", &GO->GetMaterial().DepthWriting);
		ImGui::Checkbox("Transparent",   &GO->GetMaterial().Transparent);
		ImGui::Checkbox("Lighting",      &GO->GetMaterial().Lighting);
		ImGui::Spacing();
		
		ImGui::DragFloat2("Tiling",        (float*)&GO->GetMaterial().Tiling,           0.01f);
		ImGui::DragFloat2("Detail Tiling", (float*)&GO->GetMaterial().DetailTiling,     0.01f);
		ImGui::ColorEdit4("Albedo",        (float*)&GO->GetMaterial().Albedo);
		ImGui::SliderFloat("Roughness",            &GO->GetMaterial().Roughness,        0.00f, 1.0f);
		ImGui::SliderFloat("Metallic",             &GO->GetMaterial().Metallic,         0.00f, 1.0f);
		ImGui::DragFloat("Emission Strength",      &GO->GetMaterial().EmissionStrength, 0.01f);

		ImGui::Unindent(10.0f);
	}

	void DrawComponentsEditor(GameObject* GO)
	{
		DrawComponentEditor(GO->GetComponent<ComponentLight>());
		DrawComponentEditor(GO->GetComponent<ComponentParticleSystem>());
	}

	void DrawComponentEditor(ComponentLight* Co)
	{
		if (Co != nullptr)
		{
			if (ImGui::CollapsingHeader(ICON_FA_LIGHTBULB_O" Light##PanelInspector_Light"))
			{
				auto Light = Co->GetLight();
				if (Light != nullptr)
				{
					const char* Types[] = { "Directional", "Point", "Spot"};

					ImGui::Indent(10.0f);

					ImGui::Combo("Type", &Light->Type, Types, 3);
					ImGui::ColorEdit3("Color", (float*)&Light->Color);
					ImGui::DragFloat("Range", &Light->Range, 0.1f);
					ImGui::DragFloat("Inner Cutoff", &Light->InnerCutoff, 0.01f);
					ImGui::DragFloat("Outer Cutoff", &Light->OuterCutoff, 0.01f);

					ImGui::Unindent(10.0f);
				}

				ImGui::Separator();
			}
		}
	}

	void DrawComponentEditor(ComponentParticleSystem* Co)
	{
		if (Co != nullptr)
		{
			if (ImGui::CollapsingHeader(ICON_FA_CERTIFICATE" Particle System##PanelInspector_ParticleSystem"))
			{
				const char* BlendModes[] = { "Default", "Add", "Subtract", "Multiply" };
				const char* BillboardModes[] = { "None", "Vertical", "Horizontal", "Face to camera" };
				const char* TransformationModes[] = { "World", "Local" };
				const char* SortModes[] = { "None", "Young first", "Old first", "Nearest first" };

				auto Emitter = &Co->GetEmitter();

				ImGui::Indent(10.0f);
				
				ImGui::SliderInt("Max Particles", (int*)&Emitter->MaxParticles, 0, 1024);
				ImGui::DragFloat("Emit Rate", &Emitter->EmitRate, 0.1f, 0.0f, FLT_MAX);
				ImGui::Checkbox("Emit", &Emitter->Emit);
				ImGui::Checkbox("Visible", &Emitter->Visible);

				ImGui::Combo("Blend mode", (int*)&Emitter->Blend, BlendModes, 4);
				ImGui::Combo("Billboard mode", (int*)&Emitter->Billboard, BillboardModes, 4);
				ImGui::Combo("Transformation mode", (int*)&Emitter->Transformation, TransformationModes, 2);
				ImGui::Combo("Sort mode", (int*)&Emitter->Sort, SortModes, 4);

				ImGui::Separator();

				if (ImGui::CollapsingHeader("Lifetime"))
				{
					ImGui::Indent(10.0f);
					ImGui::DragFloatRange2("Lifetime", &Emitter->ModuleLifetime.Min, &Emitter->ModuleLifetime.Max, 0.1f);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader("Location"))
				{
					const char* Shapes[] = { "Point", "Box", "Circle", "Sphere" };

					ImGui::Indent(10.0f);
					ImGui::DragFloat3("Shape Size", (float*)&Emitter->ModuleLocation.Size, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat("Shape Radius", &Emitter->ModuleLocation.Radius, 0.1f, 0.0f, FLT_MAX);
					ImGui::Combo("Shape", (int*)&Emitter->ModuleLocation.Shape, Shapes, 4);
					ImGui::Checkbox("Emit from shell", &Emitter->ModuleLocation.EmitFromShell);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader("Velocity"))
				{
					ImGui::Indent(10.0f);
					ImGui::DragFloat3("Min velocity", (float*)&Emitter->ModuleVelocity.Min, 0.1f);
					ImGui::DragFloat3("Max velocity", (float*)&Emitter->ModuleVelocity.Max, 0.1f);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader("Rotation"))
				{
					ImGui::Indent(10.0f);
					ImGui::DragFloatRange2("Initial rotation", &Emitter->ModuleRotation.Min, &Emitter->ModuleRotation.Max, 0.1f);
					ImGui::DragFloatRange2("Velocity of rotation", &Emitter->ModuleRotation.MinVelocity, &Emitter->ModuleRotation.MaxVelocity, 0.1f);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader("Color"))
				{
					const char* Modes[] = { "Initial", "Over life" };

					ImGui::Indent(10.0f);
					
					ImGui::Combo("Color mode", (int*)&Emitter->ModuleColor.Mode, Modes, 2);

					if (Emitter->ModuleColor.Mode == ParticleModuleColor::UpdateMode::Initial)
					{
						ImGui::DragFloat4("Min color", (float*)&Emitter->ModuleColor.Min);
						ImGui::DragFloat4("Max color", (float*)&Emitter->ModuleColor.Max);
					}

					if (Emitter->ModuleColor.Mode == ParticleModuleColor::UpdateMode::OverLife)
					{
						size_t Counter = 1;
						for (auto& Node : Emitter->ModuleColor.Curve.Points)
						{
							auto str = std::to_string(Counter);

							float Min = Counter == 1 ? 0.0f : Emitter->ModuleColor.Curve.Points[Counter - 2].first;
							float Max = Counter == Emitter->ModuleColor.Curve.Points.size() ? 1.0f : Emitter->ModuleColor.Curve.Points[Counter].first;

							if (ImGui::CollapsingHeader((str + " colorpoint").c_str()))
							{
								auto p = "Colorpoint " + str;
								auto c = "Color " + str;
								auto r = "Remove colorpoint " + str;

								ImGui::Indent(10.0f);
								ImGui::SetNextItemWidth(50.0f);
								ImGui::DragFloat(p.c_str(), &Node.first, 0.05f, Min, Max);
								ImGui::ColorEdit4(c.c_str(), (float*)&Node.second);
								if (ImGui::Button(r.c_str())) Emitter->ModuleColor.Curve.RemovePoint(Counter - 1);
								ImGui::Unindent(10.0f);
							}

							Counter++;
						}

						Emitter->ModuleColor.Curve.Sort();

						if (ImGui::Button("Add colorpoint"))
						{
							Emitter->ModuleColor.Curve.AddPoint(Vector4(1), 1.0f);
						}
					}

					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader("Size"))
				{
					const char* Modes[] = { "Initial", "Over life" };

					ImGui::Indent(10.0f);

					ImGui::Combo("Size mode", (int*)&Emitter->ModuleSize.Mode, Modes, 2);

					if (Emitter->ModuleSize.Mode == ParticleModuleSize::UpdateMode::Initial)
					{
						ImGui::DragFloat3("Min size", (float*)&Emitter->ModuleSize.Min);
						ImGui::DragFloat3("Max size", (float*)&Emitter->ModuleSize.Max);
					}

					if (Emitter->ModuleSize.Mode == ParticleModuleSize::UpdateMode::OverLife)
					{
						size_t Counter = 1;
						for (auto& Node : Emitter->ModuleSize.Curve.Points)
						{
							auto str = std::to_string(Counter);

							float Min = Counter == 1 ? 0.0f : Emitter->ModuleSize.Curve.Points[Counter - 2].first;
							float Max = Counter == Emitter->ModuleSize.Curve.Points.size() ? 1.0f : Emitter->ModuleSize.Curve.Points[Counter].first;

							if (ImGui::CollapsingHeader((str + " sizepoint").c_str()))
							{
								auto p = "Sizepoint " + str;
								auto c = "Size " + str;
								auto r = "Remove sizepoint " + str;

								ImGui::Indent(10.0f);
								ImGui::SetNextItemWidth(50.0f);
								ImGui::DragFloat(p.c_str(), &Node.first, 0.05f, Min, Max);
								ImGui::DragFloat3(c.c_str(), (float*)&Node.second, 0.1f, 0.0f, FLT_MAX);
								if (ImGui::Button(r.c_str())) Emitter->ModuleSize.Curve.RemovePoint(Counter - 1);
								ImGui::Unindent(10.0f);
							}

							Counter++;
						}

						Emitter->ModuleSize.Curve.Sort();

						if (ImGui::Button("Add sizepoint"))
						{
							Emitter->ModuleSize.Curve.AddPoint(Vector3(1), 1.0f);
						}
					}

					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader("Noise"))
				{
					ImGui::Indent(10.0f);

					ImGui::DragFloat("Noise strength", &Emitter->ModuleNoise.Strength, 0.1f, 0.0f, FLT_MAX);
					ImGui::SliderInt("Noise octaves", &Emitter->ModuleNoise.Octaves, 1, 8);
					ImGui::DragFloat("Noise lacunarity", &Emitter->ModuleNoise.Lacunarity, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat("Noise persistence", &Emitter->ModuleNoise.Persistence, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat("Noise frequency", &Emitter->ModuleNoise.Frequency, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat("Noise amplitude", &Emitter->ModuleNoise.Amplitude, 0.1f, 0.0f, FLT_MAX);

					ImGui::Unindent(10.0f);
				}

				if (ImGui::CollapsingHeader("SubUV"))
				{
					const char* Modes[] =  { "Linear", "Random" };

					ImGui::Indent(10.0f);

					ImGui::DragInt("Horizontal", &Emitter->ModuleSubUV.Horizontal, 1, 0, INT_MAX);
					ImGui::DragInt("Vertical", &Emitter->ModuleSubUV.Vertical, 1, 0, INT_MAX);
					ImGui::DragFloat("Cycles", &Emitter->ModuleSubUV.Cycles, 0.1f, 0.0f, FLT_MAX);
					ImGui::Combo("SubUV mode", (int*)&Emitter->ModuleSubUV.Mode, Modes, 2);

					ImGui::Unindent(10.0f);
				}

				ImGui::Unindent(10.0f);
				ImGui::Separator();
			}
		}
	}

}


