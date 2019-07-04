#include <Editor/PanelHierarchy.h>
#include <Editor/FontAwesome.h>
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>

#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentLight.h>
#include <algorithm>
#include <string>

namespace Columbus
{

	static void CopyComponent(ComponentMeshRenderer* Co, GameObject& GO)
	{
		if (Co != nullptr)
		{
			GO.AddComponent(new ComponentMeshRenderer(Co->GetMesh()));
		}
	}

	static void CopyComponent(ComponentLight* Co, GameObject& GO)
	{
		if (Co != nullptr)
		{
			GO.AddComponent(new ComponentLight(new Light(*Co->GetLight())));
		}
	}

	void EditorPanelHierarchy::Draw()
	{
		static std::string Find;

		if (Opened && scene != nullptr)
		{
			if (ImGui::Begin(ICON_FA_LIST_UL" Hierarchy##PanelHierarchy", &Opened, ImGuiWindowFlags_NoCollapse))
			{
				bool Delete = false;
				std::string DeleteName;

				if (ImGui::BeginChild("##Find_PanelHierarchy", ImVec2(ImGui::GetWindowContentRegionWidth(), 20)))
				{
					ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
					ImGui::InputText("##Find_PanelHierarchy_Find", &Find);
				}
				ImGui::EndChild();


				if (ImGui::BeginChild("##List_PanelHierarchy"))
				{
					std::string MFind = Find;
					std::string MName;
					std::transform(MFind.begin(), MFind.end(), MFind.begin(), ::tolower);

					for (auto& Obj : scene->Objects.Resources)
					{
						MName = Obj->Name;
						std::transform(MName.begin(), MName.end(), MName.begin(), ::tolower);

						if (MName.find(MFind) != std::string::npos)
						{
							if (ImGui::Selectable(Obj->Name.c_str(), object == Obj.Get()))
							{
								object = Obj.Get();
							}

							// if (ctr+c)
							if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C), false))
							{
								buffer = object;
							}

							// if (delete)
							if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)) && !Delete)
							{
								if (object != nullptr)
								{
									Delete = true;
									DeleteName = object->Name;
								}
							}
						}
					}
				}
				ImGui::EndChild();

				if (Delete)
				{
					if (object == buffer)
						buffer = nullptr;

					object = nullptr;
					scene->Objects.Delete(DeleteName);
				}
			}
			ImGui::End();

			// if(ctrl+v)
			if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V), true))
			{
				if (buffer != nullptr)
				{
					GameObject tmp;
					tmp.transform = buffer->transform;
					tmp.transform.Position = Vector3::Random({-10}, {10});
					tmp.Name = buffer->Name + " ";
					tmp.Enable = buffer->Enable;
					tmp.materialID = buffer->materialID;

					CopyComponent((ComponentMeshRenderer*)buffer->GetComponent(Component::Type::MeshRenderer), tmp);
					CopyComponent((ComponentLight*)buffer->GetComponent(Component::Type::Light), tmp);

					std::string Name;

					for (uint32 i = 0;; i++)
					{
						Name = tmp.Name + std::to_string(i);
						if (scene->Objects.Find(Name) == nullptr)
						{
							tmp.Name = Name;
							break;
						}
					}

					scene->Add(std::move(tmp));
				}
			}
		}
	}

}


