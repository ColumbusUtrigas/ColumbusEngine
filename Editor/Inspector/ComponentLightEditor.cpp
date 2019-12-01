#include <Editor/Inspector/PanelInspector.h>
#include <Editor/Icons.h>
#include <Scene/ComponentLight.h>
#include <Lib/imgui/imgui.h>

namespace Columbus
{

	void EditorPanelInspector::DrawComponentLightEditor(Scene& Scn)
	{
		auto Co = (ComponentLight*)Inspectable->GetComponent(Component::Type::Light);

		if (Co != nullptr)
		{
			if (ImGui::CollapsingHeader(LIGHT_ICON" Light##PanelInspector_Light"))
			{
				auto Light = Co->GetLight();
				
				if (Light != nullptr)
				{
					const char* Types[] = { "Directional", "Point", "Spot"};

					ImGui::Indent(10.0f);

					ImGui::Combo("Type##PanelInspector_Light",               &Light->Type, Types, 3);
					ImGui::Checkbox("Shadows##PanelInspector_Light",         &Light->Shadows);
					ImGui::ColorEdit3("Color##PanelInspector_Light", (float*)&Light->Color);
					ImGui::DragFloat("Energy##PanelInspector_Light",         &Light->Energy, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat("Range##PanelInspector_Light",          &Light->Range,  0.1f);
					ImGui::DragFloat("Inner Cutoff##PanelInspector_Light",   &Light->InnerCutoff, 0.01f);
					ImGui::DragFloat("Outer Cutoff##PanelInspector_Light",   &Light->OuterCutoff, 0.01f);

					ImGui::Unindent(10.0f);
				}

				ImGui::Separator();
			}
		}
	}

}


