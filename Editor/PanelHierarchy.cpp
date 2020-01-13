#include <Editor/PanelHierarchy.h>
#include <Editor/FontAwesome.h>
#include <Lib/imgui/imgui.h>

#include <SDL.h>

namespace Columbus
{

	void EditorPanelHierarchy::Draw()
	{
		static String Find;

		if (Opened && scene != nullptr)
		{
			if (ImGui::Begin(ICON_FA_LIST_UL" Hierarchy##PanelHierarchy", &Opened, ImGuiWindowFlags_NoCollapse))
			{
				bool Delete = false;
				String DeleteName;

				if (ImGui::BeginChild("##Find_PanelHierarchy", ImVec2(ImGui::GetWindowContentRegionWidth(), 20)))
				{
					ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
					char tmp[1024];
					memcpy(tmp, Find.data(), Find.size() + 1);
					ImGui::InputText("##Find_PanelHierarchy_Find", tmp, 1024);
					Find = tmp;
				}
				ImGui::EndChild();


				if (ImGui::BeginChild("##List_PanelHierarchy"))
				{
					String MFind = str_tolower(Find);
					String MName;

					for (auto& Obj : scene->Objects.Resources)
					{
						MName = str_tolower(Obj->Name);

						if (MName.find(MFind) != String::npos)
						{
							auto selected = ImGui::Selectable(Obj->Name.c_str(), object == Obj.Get());
							if (selected || ImGui::IsItemFocused())
							{
								object = Obj.Get();
							}

							// if (ctrl+c)
							if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C), false))
							{
								buffer = object;
							}

							// if (ctrl+d)
							if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(SDL_SCANCODE_D, false))
							{
								buffer2 = object;
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

			bool ctrlD = ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(SDL_SCANCODE_D, false);

			// if(ctrl+v || ctrl+d)
			if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V), true) || ctrlD)
			{
				auto buf = ctrlD ? buffer2 : buffer;
				if (buf != nullptr)
				{
					auto tmp = std::move(*buf->Clone());

					for (int i = tmp.Name.size() - 1; i >= 0; i--)
					{
						if (!isdigit(tmp.Name[i]) && !isspace(tmp.Name[i])) break;
						tmp.Name.pop_back();
					}

					tmp.Name += ' ';

					String Name;

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
					object = scene->Objects.Find(Name);
				}
			}
		}
	}

}


