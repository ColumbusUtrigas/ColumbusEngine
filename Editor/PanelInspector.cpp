#include <Editor/PanelInspector.h>
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>

namespace Columbus
{

	static void DrawTransformEditor(GameObject* GO);
	static void DrawMaterialEditor(GameObject* GO);

	void EditorPanelInspector::Draw()
	{
		if (Opened)
		{
			if (ImGui::Begin(Name.c_str(), &Opened, ImGuiWindowFlags_NoCollapse))
			{
				if (Inspectable != nullptr)
				{
					ImGui::InputText(" ", &Inspectable->Name);
					ImGui::Separator();

					if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
					{
						DrawTransformEditor(Inspectable);
						ImGui::Separator();
					}

					if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
					{
						DrawMaterialEditor(Inspectable);
						ImGui::Separator();
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

}


