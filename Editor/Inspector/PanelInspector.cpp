#include <Editor/Inspector/PanelInspector.h>
#include <Editor/Icons.h>
#include <Scene/ComponentAudioSource.h>
#include <Scene/ComponentLight.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/ComponentRigidbody.h>
#include <Scene/ComponentBillboard.h>
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>
#include <unordered_map>
#include <functional>
#include <tuple>

using namespace std;

namespace Columbus
{

	int gTypeId = 0;
	template <typename T>
	int type_id()
	{
		static int id = ++gTypeId;
		return id;
	}

#define ELEM(t, n, ...) { type_id<t>(), { \
	n"##PanelInspector_ModalWindow_AddComponent", \
	[](GameObject* GO) { GO->AddComponent<t>(__VA_ARGS__); }, \
	[](GameObject* GO) { return GO->HasComponent<t>(); } \
}}

	unordered_map<int, tuple<string, function<void(GameObject*)>, function<bool(GameObject*)>>> names
	{
		ELEM(ComponentAudioSource, AUDIO_ICON" Audio Source", make_shared<AudioSource>()),
		ELEM(ComponentLight, LIGHT_ICON" Light", new Light()),
		ELEM(ComponentMeshRenderer, MESH_ICON" Mesh Renderer", nullptr),
		ELEM(ComponentParticleSystem, PARTICLES_ICON" Particle System", ParticleEmitterCPU{}),
		ELEM(ComponentRigidbody, RIGIDBODY_ICON" Rigidbody", new Rigidbody()),
		ELEM(ComponentBillboard, "Billboard")
	};

	void EditorPanelInspector::Draw(Scene& Scn)
	{
		if (Opened)
		{
			if (ImGui::Begin(ICON_FA_INFO" Inspector##PanelInspector", &Opened, ImGuiWindowFlags_NoCollapse))
			{
				if (Inspectable != nullptr)
				{
					string Tmp = Inspectable->Name.c_str();

					ImGui::Checkbox("##PanelInspector_Enable", &Inspectable->Enable);
					ImGui::SameLine();
					ImGui::InputText("##PanelInspector_Name", &Tmp);
					ImGui::Separator();

					if (Inspectable->Name != Tmp.c_str())
					{
						Scn.Objects.Rename(Inspectable->Name, Tmp.c_str());
						Inspectable->Name = Tmp.c_str();
					}

					// Draw transform editor
					if (ImGui::CollapsingHeader(TRANSFORM_ICON" Transform##PanelInspector_Transform", ImGuiTreeNodeFlags_DefaultOpen))
					{
						DrawTransformEditor();
						ImGui::Separator();
					}

					//Draw material editor
					DrawMaterialEditor(Scn);

					// Draw components editor and "Add component" button
					DrawComponentsEditor(Scn);
					DrawAddComponent(Scn);
				}
			}
			ImGui::End();
		}
	}

	EditorPanelInspector::~EditorPanelInspector() {}

	void Sel(GameObject* GO, int id, int& Selected, std::function<void()> Close)
	{
		ImGuiSelectableFlags Flags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_DontClosePopups;
		ImVec2 Size = ImVec2(0, 50);

		if (ImGui::Selectable(get<0>(names[id]).c_str(), Selected == id, Flags | (get<2>(names[id])(GO) ? ImGuiSelectableFlags_Disabled : 0), Size))
		{
			Selected = id;
			if (ImGui::IsMouseDoubleClicked(0)) { get<1>(names[id])(GO); Close(); }
		}
		ImGui::Separator();
	}

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
		GameObject*& GO = Inspectable;

		auto Close = [&]() {
			AddComponentEnable = false;
			Selected = 0;
		};

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
					for (const auto& tup : names)
						Sel(GO, tup.first, Selected, Close);
				}
				ImGui::EndChild();

				if (ImGui::BeginChild("Buttons##PanelInspector_ModalWindow_Buttons"))
				{
					if (ImGui::Button("Cancel##PanelInspector_ModalWindow_Cancel")) Close();
					ImGui::SameLine();
					if (ImGui::Button("Add##PanelInspector_ModalWindow_AddComponent_Add")) get<1>(names[Selected])(GO);
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
		DrawComponentBillboardEditor(Scn);
	}

}
