#include <Editor/PanelHierarchy.h>
#include <Editor/FontAwesome.h>
#include <Editor/Icons.h>
#include <Lib/imgui/imgui.h>
#include <functional>

#include <SDL.h>

namespace Columbus::Editor
{

	PanelHierarchy::PanelHierarchy() : Panel(ICON_FA_LIST_UL" Hierarchy") {}
	PanelHierarchy::~PanelHierarchy() {}

	void PanelHierarchy::DrawInternal()
	{
		static std::string Find;

		if (scene != nullptr)
		{
			bool Delete = false;
			std::string DeleteName;

			if (ImGui::BeginChild("##Find_PanelHierarchy", ImVec2(ImGui::GetWindowContentRegionWidth(), 20)))
			{
				ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth()-30);
				char tmp[1024];
				memcpy(tmp, Find.data(), Find.size() + 1);
				ImGui::InputText("##Find_PanelHierarchy_Find", tmp, 1024);
				ImGui::SameLine();
				if (ImGui::Button(DELETE_ICON))
					memset(tmp, 0, 1024);
				Find = tmp;
			}
			ImGui::EndChild();

			if (ImGui::Button("Add Empty"))
				scene->AddEmpty();

			if (ImGui::BeginChild("##List_PanelHierarchy"))
			{
				std::string MFind = str_tolower(Find);
				std::string MName;

				auto src_dragndrop = [](GameObject* obj) {
					if (ImGui::BeginDragDropSource()) {
						ImGui::SetDragDropPayload("ptr", obj, sizeof(obj));
						ImGui::Text(obj->Name.c_str());
						ImGui::EndDragDropSource();
					}
				};
				auto dst_dragndrop = [&](GameObject* obj) {
					if (ImGui::BeginDragDropTarget()) {
						if (auto ptr = ImGui::AcceptDragDropPayload("ptr"))
						{
							auto c = obj->AddChild(object);
						}

						ImGui::EndDragDropTarget();
					}
				};

				auto do_object_logic = [&](GameObject* obj) {
					// if (ctrl+c)
					if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C), false)) {
						buffer = object;
					}

					// if (ctrl+d)
					if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(SDL_SCANCODE_D, false)) {
						buffer2 = object;
					}

					// if (delete)
					if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)) && !Delete && object != nullptr) {
						Delete = true;
						DeleteName = object->Name;
					}
				};

				std::function<void(GameObject* obj, bool drawLeafs)> draw_object_leaf;
				draw_object_leaf = [&](GameObject* obj, bool drawLeafs) {
					int flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
					if (object == obj) flags |= ImGuiTreeNodeFlags_Selected;
					if (obj->GetChildren().empty()) flags |= ImGuiTreeNodeFlags_Leaf;

					bool open = ImGui::TreeNodeEx(obj->Name.c_str(), flags);
					if (ImGui::IsItemClicked() || ImGui::IsItemFocused()) object = obj;
					src_dragndrop(obj);
					dst_dragndrop(obj);
					if (open) {
						for (const auto& child : obj->GetChildren())
							draw_object_leaf(child, drawLeafs);
						ImGui::TreePop();
					}

					do_object_logic(obj);
				};

				if (MFind.empty())
				{
					for (auto& Obj : scene->Objects.Resources)
					{
						if (Obj->GetParent() != nullptr) continue;
						draw_object_leaf(Obj.Get(), true);
					}
				}
				else
				{
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

							do_object_logic(object);
						}
					}
				}

				/*for (auto& Obj : scene->Objects.Resources)
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
				}*/
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


