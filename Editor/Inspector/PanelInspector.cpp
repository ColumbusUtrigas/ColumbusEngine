#include <Editor/Inspector/PanelInspector.h>
#include <Editor/Icons.h>
#include <Scene/ComponentAudioSource.h>
#include <Scene/ComponentLight.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/ComponentRigidbody.h>
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>

namespace Columbus
{

	void EditorPanelInspector::Draw(Scene& Scn)
	{
		if (Opened)
		{
			if (ImGui::Begin(ICON_FA_INFO" Inspector##PanelInspector", &Opened, ImGuiWindowFlags_NoCollapse))
			{
				if (Inspectable != nullptr)
				{
					ImGui::Checkbox("##PanelInspector_Enable", &Inspectable->Enable);
					ImGui::SameLine();
					ImGui::InputText("##PanelInspector_Name", &Inspectable->Name);
					ImGui::Separator();

					// Draw transform editor
					if (ImGui::CollapsingHeader(TRANSFORM_ICON" Transform##PanelInspector_Transform", ImGuiTreeNodeFlags_DefaultOpen))
					{
						DrawTransformEditor();
						ImGui::Separator();
					}

					//Draw material editor
					if (ImGui::CollapsingHeader(MATERIAL_ICON" Material##PanelInspector_Material"))
					{
						DrawMaterialEditor();
						ImGui::Separator();
					}

					// Draw components editor and "Add component" button
					DrawComponentsEditor(Scn);
					DrawAddComponent(Scn);
				}
			}
			ImGui::End();
		}
	}



	EditorPanelInspector::~EditorPanelInspector() {}



	void EditorPanelInspector::DrawAddComponent(Scene& Scn)
	{
		static int Selected = 0;
		static bool AddComponentEnable = false;

		ImGui::Dummy(ImVec2(0, 10));
		if (ImGui::Button("Add##PanelInspector_AddComponent", ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
		{
			AddComponentEnable = true;
		}

		const char* Name = "Add Component##PanelInspector_ModalWindow_AddComponent";
		const char* NameAudio = AUDIO_ICON" Audio Source##PanelInspector_ModalWindow_AddComponent_Audio";
		const char* NameLight = LIGHT_ICON" Light##PanelInspector_ModalWindow_AddComponent_Light";
		const char* NameMesh = MESH_ICON" Mesh Renderer##PanelInspector_ModalWindow_AddComponent_Mesh";
		const char* NameParticles = PARTICLES_ICON" Particle Emitter##PanelInspector_ModalWindow_AddComponent_Particles";
		const char* NameRigidbody = RIGIDBODY_ICON" Rigidbody##PanelInspector_ModalWindow_AddComponent_Rigidbody";

		GameObject*& GO = Inspectable;

		// Flags for components' selectables and its size
		ImGuiSelectableFlags Flags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_DontClosePopups;
		ImVec2 Size = ImVec2(0, 50);

		auto Close = [&]()
		{
			AddComponentEnable = false;
			Selected = 0;
		};

		auto Add = [&]()
		{
			switch (Selected)
			{
			case 1: GO->AddComponent(new ComponentAudioSource(new AudioSource())); break;
			case 2:
				GO->AddComponent(new ComponentLight(new Light));
				//GO->LightIndex = Scn.Lights.size();
				//Scn.Lights.push_back(new Light());
				break;
			case 3: GO->AddComponent(new ComponentMeshRenderer(nullptr)); break;
			case 4: GO->AddComponent(new ComponentParticleSystem(ParticleEmitterCPU())); break;
			case 5: GO->AddComponent(new ComponentRigidbody(new Rigidbody())); break;
			default: break;
			}

			Close();
		};

		// Draw selectable for "Add component"
		auto Selectable = [&](const char* Name, int ID, bool Enable)
		{
			if (ImGui::Selectable(Name, Selected == ID, Flags | (Enable ? ImGuiSelectableFlags_Disabled : 0), Size))
			{
				Selected = ID;
				if (ImGui::IsMouseDoubleClicked(0)) Add();
			}
			ImGui::Separator();
		};

		bool IsAudio = GO->HasComponent(Component::Type::AudioSource);
		bool IsLight = GO->HasComponent(Component::Type::Light);
		bool IsMesh = GO->HasComponent(Component::Type::MeshRenderer);
		bool IsParticles = GO->HasComponent(Component::Type::ParticleSystem);
		bool IsRigidbody = GO->HasComponent(Component::Type::Rigidbody);

		if (AddComponentEnable)
		{
			if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape))) Close();

			ImGui::OpenPopup(Name);
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSizeConstraints(ImVec2(450, 250), ImVec2(FLT_MAX, FLT_MAX));
			if (ImGui::BeginPopupModal(Name, &AddComponentEnable, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
			{
				ImVec2 size = ImVec2(ImGui::GetContentRegionMax().x, ImGui::GetContentRegionMax().y - 50.0f);
				if (ImGui::BeginChild("Components List##PanelInspector_ModalWindow_List", size))
				{
					Selectable(NameAudio, 1, IsAudio);
					Selectable(NameLight, 2, IsLight);
					Selectable(NameMesh, 3, IsMesh);
					Selectable(NameParticles, 4, IsParticles);
					Selectable(NameRigidbody, 5, IsRigidbody);
				}
				ImGui::EndChild();

				if (ImGui::BeginChild("Buttons##PanelInspector_ModalWindow_Buttons"))
				{
					if (ImGui::Button("Cancel##PanelInspector_ModalWindow_Cancel")) Close();
					ImGui::SameLine();
					if (ImGui::Button("Add##PanelInspector_ModalWindow_AddComponent_Add")) Add();
				}
				ImGui::EndChild();
				ImGui::EndPopup();
			}
		}
	}



	void EditorPanelInspector::DrawComponentsEditor(Scene& Scn)
	{
		DrawComponentAudioEditor(Scn);
		DrawComponentLightEditor(Scn);
		DrawComponentMeshRendererEditor(Scn);
		DrawComponentParticleSystemEditor(Scn);
		DrawComponentRigidbodyEditor(Scn);
	}

}


