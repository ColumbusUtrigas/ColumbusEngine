#pragma once

#include <Editor/Inspector/ComponentEditor.h>
#include <Scene/ComponentLight.h>
#include <imgui/imgui.h>

namespace Columbus::Editor
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

			ImGui::Combo("Type",               &l.Type, Types, 3);
			ImGui::Checkbox("Shadows",         &l.Shadows);
			ImGui::ColorEdit3("Color", (float*)&l.Color);
			ImGui::DragFloat("Energy",         &l.Energy, 0.1f, 0.0f, FLT_MAX);
			ImGui::DragFloat("Range",          &l.Range, 0.1f);
			ImGui::DragFloat("Inner Cutoff",   &l.InnerCutoff, 0.01f);
			ImGui::DragFloat("Outer Cutoff",   &l.OuterCutoff, 0.01f);
		}
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentLight, ComponentEditorLight);

}
