#include <Editor/Inspector/ComponentEditor.h>
#include <Scene/ComponentAudioSource.h>
#include <imgui/imgui.h>

namespace Columbus::Editor
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

				ImGui::Combo("Type",       (int*)&Source->SoundMode, Types, 2);
				ImGui::DragFloat("Gain",         &Source->Gain, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Pitch",        &Source->Pitch, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Min Distance", &Source->MinDistance, 0.1f, 0.1f, 1000.0f);
				ImGui::DragFloat("Max Distance", &Source->MaxDistance, 0.1f, 1.0f, 1000.0f);
				ImGui::DragFloat("Rolloff",      &Source->Rolloff, 0.1f, 0.0f, 10.0f);
				ImGui::Checkbox("Play",          &Source->Playing);
				ImGui::Checkbox("Loop",          &Source->Looping);
			}
		}
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentAudioSource, ComponentEditorAudio);

}
