#include <Editor/Inspector/PanelInspector.h>
#include <Editor/Inspector/ComponentEditor.h>
#include <Editor/Icons.h>
#include <Editor/CommonUI.h>
#include <Scene/Component.h>
#include <Scene/ComponentRigidbody.h>
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/imgui_internal.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>
#include <unordered_map>
#include <functional>
#include <tuple>
#include <map>

using namespace std;

namespace Columbus::Editor
{

	static auto InspectorComponentNameRelocate(string_view str) {
		static unordered_map<string_view, string_view> reloc {
			{ "ComponentAudioSource", AUDIO_ICON" Audio Source" },
			{ "ComponentLight", LIGHT_ICON" Light" },
			{ "ComponentMeshRenderer", MESH_ICON" Mesh Renderer" },
			{ "ComponentParticleSystem", PARTICLES_ICON" Particle System" },
			{ "ComponentRigidbody", RIGIDBODY_ICON" Rigidbody" }
		};

		auto it = reloc.find(str);
		if (it != reloc.end()) return it->second;
		return str;
	}

	void PanelInspector::Draw(Scene& Scn)
	{
		if (Opened)
		{
			if (ImGui::Begin(ICON_FA_INFO" Inspector##PanelInspector", &Opened, ImGuiWindowFlags_NoCollapse))
			{
				if (Inspectable != nullptr)
				{
					auto RB = (ComponentRigidbody*)Inspectable->GetComponent(Component::Type::Rigidbody);
					if (RB != nullptr && RB->GetRigidbody() != nullptr)
					{
						RB->GetRigidbody()->Activate();
					}

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

	PanelInspector::~PanelInspector() {}

	void Select(GameObject* GO, string_view Name, Component* Comp, Component*& Selected, function<void()> Close)
	{
		ImGuiSelectableFlags flags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_DontClosePopups;
		ImVec2 size = ImVec2(0, 50);
		if (GO->HasComponent(Comp->GetType())) flags |= ImGuiSelectableFlags_Disabled;

		if (ImGui::Selectable(Name.data(), Comp == Selected, flags, size))
		{
			Selected = Comp;
			if (ImGui::IsMouseDoubleClicked(0)) { GO->AddComponent(Comp->Clone()); Close(); }
		}
		ImGui::Separator();
	}

	void PanelInspector::DrawAddComponent(Scene& Scn)
	{
		static Component* selected = nullptr;
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
			selected = nullptr;
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
					auto builders = PrototypeFactory<Component>::Instance().GetBuilders();
					std::map<std::string, Component*> components(builders.begin(), builders.end());

					for (const auto& comp : components)
					{
						Select(GO, InspectorComponentNameRelocate(comp.first), comp.second, selected, Close);
					}
				}
				ImGui::EndChild();

				if (ImGui::BeginChild("Buttons##PanelInspector_ModalWindow_Buttons"))
				{
					if (ImGui::Button("Cancel##PanelInspector_ModalWindow_Cancel")) Close();
					ImGui::SameLine();

					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, selected == nullptr);
					if (ImGui::Button("Add##PanelInspector_ModalWindow_AddComponent_Add") && selected)
						GO->AddComponent(selected->Clone());
					ImGui::PopItemFlag();
				}
				ImGui::EndChild();
				ImGui::EndPopup();
			}
		}
	}


	void PanelInspector::DrawComponentsEditor(Scene& Scn)
	{
		for (const auto& comp : Inspectable->GetComponents()) {
			auto name = InspectorComponentNameRelocate(comp->GetTypename()).data();
			if (ImGui::CollapsingHeader(name)) {
				auto& ed = PrototypeFactory<ComponentEditor>::Instance().GetFromTypename(comp->GetTypename());
				ed.Target = comp.Get();
				
				ImGui::Indent(10);
				ImGui::PushID(ed.GetTypename().data());
				ed.OnInspectorGUI();
				ImGui::PopID();
				ImGui::Unindent(10);
				ImGui::Separator();
			}
		}
	}

}
