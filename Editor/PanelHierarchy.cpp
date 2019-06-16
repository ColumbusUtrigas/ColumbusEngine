#include <Editor/PanelHierarchy.h>
#include <Editor/FontAwesome.h>
#include <Lib/imgui/imgui.h>

#include <Scene/ComponentMeshRenderer.h>

namespace Columbus
{

	static ComponentMeshRenderer* CopyComponent(ComponentMeshRenderer* Co)
	{
		if (Co != nullptr)
		{
			return new ComponentMeshRenderer(Co->GetMesh());
		}

		return nullptr;
	}

	void EditorPanelHierarchy::Draw()
	{
		if (Opened && scene != nullptr)
		{
			if (ImGui::Begin((ICON_FA_LIST_UL" " + Name + "##PanelHierarchy").c_str(), &Opened, ImGuiWindowFlags_NoCollapse))
			{
				for (auto& Obj : scene->Objects)
				{
					if (ImGui::Selectable(Obj.second->Name.c_str(), object == Obj.second.Get()))
					{
						object = Obj.second.Get();
					}

					// if (ctr+c)
					if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C), false))
					{
						buffer = object;
					}
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
						tmp.SetMaterial(buffer->GetMaterial());
						tmp.GetMaterial().Albedo = Vector4(Vector3::Random({0.0f}, {1.0f}), 1);
						tmp.GetMaterial().Roughness = Random::Range(0.0f, 1.0f);
						tmp.GetMaterial().Metallic = Random::Range(0.0f, 1.0f);

						tmp.AddComponent(CopyComponent(buffer->GetComponent<ComponentMeshRenderer>()));

						for (uint32 i = 0;; i++)
						{
							if (scene->GetGameObject(tmp.Name + std::to_string(i)) == nullptr)
							{
								tmp.Name += std::to_string(i);
								break;
							}
						}

						scene->Add(scene->Objects.size(), std::move(tmp));
					}
				}
			}
		}
	}

}


