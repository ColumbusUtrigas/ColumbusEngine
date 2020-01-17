#include <Editor/Inspector/ComponentEditor.h>
#include <Scene/ComponentAudioSource.h>
#include <imgui/imgui.h>

namespace Columbus
{

	class ComponentEditorAudio : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentAudioSource, ComponentEditorAudio);
	public:
		void OnInspectorGUI() final override
		{
			auto Source = static_cast<ComponentAudioSource*>(Target)->GetSource();
			if (Source != nullptr)
			{
				const char* Types[] = { "2D", "3D" };

				ImGui::Indent(10.0f);

				ImGui::Combo("Type##PanelInspector_Audio",       (int*)&Source->SoundMode, Types, 2);
				ImGui::DragFloat("Gain##PanelInspector_Audio",         &Source->Gain, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Pitch##PanelInspector_Audio",        &Source->Pitch, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Min Distance##PanelInspector_Audio", &Source->MinDistance, 0.1f, 0.1f, 1000.0f);
				ImGui::DragFloat("Max Distance##PanelInspector_Audio", &Source->MaxDistance, 0.1f, 1.0f, 1000.0f);
				ImGui::DragFloat("Rolloff##PanelInspector_Audio",      &Source->Rolloff, 0.1f, 0.0f, 10.0f);
				ImGui::Checkbox("Play##PanelInspector_Audio",          &Source->Playing);
				ImGui::Checkbox("Loop##PanelInspector_Audio",          &Source->Looping);

				ImGui::Unindent(10.0f);
			}

			ImGui::Separator();
		}
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentAudioSource, ComponentEditorAudio);

}
