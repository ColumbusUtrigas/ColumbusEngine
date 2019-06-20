#include <Editor/PanelHierarchy.h>
#include <Editor/FontAwesome.h>
#include <Lib/imgui/imgui.h>

#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentLight.h>

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
		if (Opened && scene != nullptr)
		{
			if (ImGui::Begin((ICON_FA_LIST_UL" " + Name + "##PanelHierarchy").c_str(), &Opened, ImGuiWindowFlags_NoCollapse))
			{
				bool Delete = false;
				std::string DeleteName;

				for (auto& Obj : scene->Objects.Resources)
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
					for (int i = 0; i < 100; i++)
					{
						GameObject tmp;
						tmp.transform = buffer->transform;
						tmp.transform.Position = Vector3::Random({-10}, {10});
						tmp.Name = buffer->Name + " ";
						tmp.Enable = buffer->Enable;
						tmp.material = buffer->material;
						tmp.material.Albedo = Vector4(Vector3::Random({0.0f}, {1.0f}), 1);
						tmp.material.Roughness = Random::Range(0.0f, 1.0f);
						tmp.material.Metallic = Random::Range(0.0f, 1.0f);

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

}


