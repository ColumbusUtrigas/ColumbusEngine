#pragma once

#include <Editor/Inspector/ComponentEditor.h>
#include <Scene/ComponentLight.h>
#include <imgui/imgui.h>

namespace Columbus
{

	class ComponentEditorLight : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentLight, ComponentEditorLight);
	public:
		void OnInspectorGUI() final override
		{
			auto comp = static_cast<ComponentLight*>(Target);
			auto& l = comp->GetLight();

			const char* Types[] = { "Directional", "Point", "Spot" };

			ImGui::Indent(10.0f);

			ImGui::Combo("Type##PanelInspector_Light",               &l.Type, Types, 3);
			ImGui::Checkbox("Shadows##PanelInspector_Light",         &l.Shadows);
			ImGui::ColorEdit3("Color##PanelInspector_Light", (float*)&l.Color);
			ImGui::DragFloat("Energy##PanelInspector_Light",         &l.Energy, 0.1f, 0.0f, FLT_MAX);
			ImGui::DragFloat("Range##PanelInspector_Light",          &l.Range, 0.1f);
			ImGui::DragFloat("Inner Cutoff##PanelInspector_Light",   &l.InnerCutoff, 0.01f);
			ImGui::DragFloat("Outer Cutoff##PanelInspector_Light",   &l.OuterCutoff, 0.01f);

			ImGui::Unindent(10.0f);
			ImGui::Separator();
		}
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentLight, ComponentEditorLight);

}
