#include <Editor/Inspector/PanelInspector.h>
#include <Editor/ResourcesViewerMaterial.h>
#include <Editor/ResourcesViewerTexture.h>
#include <Editor/ResourcesViewerShader.h>
#include <Editor/CommonUI.h>
#include <Editor/Icons.h>
#include <Core/Core.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>

namespace Columbus
{

	bool CreateNewMaterialOpen = false;
	Material** CreateNewMaterialMat = nullptr;

	static void CreateNewMaterialDraw(ResourceManager<Material>& MaterialsManager);

	void EditorPanelInspector::DrawTransformEditor()
	{
		ImGui::Indent(10.0f);

		if (ImGui::Button("Reset"))
		{
			Inspectable->transform.Position = {};
			Inspectable->transform.Rotation = {};
			Inspectable->transform.Scale = { 1,1,1 };
		}

		if (ImGui::Button("P")) Inspectable->transform.Position = {};
		ImGui::SameLine();
		ImGui::DragFloat3("Position", (float*)&Inspectable->transform.Position, 0.1f);

		if (ImGui::Button("R")) Inspectable->transform.Rotation = {};
		ImGui::SameLine();
		auto euler = Inspectable->transform.Rotation.Euler();
		ImGui::DragFloat3("Rotation", (float*)&euler, 0.5f);
		Inspectable->transform.Rotation = Quaternion(euler);

		if (ImGui::Button("S")) Inspectable->transform.Scale = { 1,1,1 };
		ImGui::SameLine();
		ImGui::DragFloat3("Scale",    (float*)&Inspectable->transform.Scale,    0.1f);

		ImGui::Unindent(10.0f);
	}



	void EditorPanelInspector::DrawMaterialEditor(Scene& Scn)
	{
		const char* CullItems[] = { "No", "Front", "Back", "Front and back"};
		const char* DepthItems[] = { "Less", "Greater", "LEqual", "GEqual", "Equal", "Not equal", "Never", "Always" };
		const char* EnvMapItems[] = { "None", "Sky", "Auto" };

		for (int i = 0; i < Inspectable->materials.size(); i++)
		{
			auto& mat = Inspectable->materials[i];
			bool Collapsing = false;

			ImGui::PushID(i);

			if (mat == nullptr)
				Collapsing = ImGui::CollapsingHeader(MATERIAL_ICON" Null Material");
			else
				Collapsing = ImGui::CollapsingHeader((MATERIAL_ICON" " + mat->Name).c_str());

			if (Collapsing)
			{
				ImGui::Indent(10.0f);

				if (ImGui::Button("Select"))
				{
					ResourcesViewerMaterial::Open(&mat);
				}

				ImGui::SameLine();

				if (ImGui::Button("New"))
				{
					CreateNewMaterialOpen = true;
					CreateNewMaterialMat = &mat;
				}

				if (mat != nullptr)
				{
					Material& material = *mat;

					ImGui::Spacing();

					ImGui::Combo("Culling", (int*)&material.Culling, CullItems, sizeofarray(CullItems));
					ImGui::Combo("Depth test", (int*)&material.DepthTesting, DepthItems, sizeofarray(DepthItems));
					ImGui::Combo("Env map mode", (int*)&material.EnvMapMode, EnvMapItems, sizeofarray(EnvMapItems));
					ImGui::Spacing();

					ImGui::Checkbox("Depth writing", &material.DepthWriting);
					ImGui::Checkbox("Transparent", &material.Transparent);
					ImGui::Checkbox("Lighting", &material.Lighting);
					ImGui::Spacing();

					ImGui::DragFloat2("Tiling", (float*)&material.Tiling, 0.01f);
					ImGui::DragFloat2("Detail Tiling", (float*)&material.DetailTiling, 0.01f);
					ImGui::ColorEdit4("Albedo", (float*)&material.Albedo);
					ImGui::SliderFloat("Roughness", &material.Roughness, 0.00f, 1.0f);
					ImGui::SliderFloat("Metallic", &material.Metallic, 0.00f, 1.0f);
					ImGui::DragFloat("Emission Strength", &material.EmissionStrength, 0.01f);
					ImGui::Spacing();

					if (ImGui::Button("Shader", ImVec2(ImGui::GetContentRegionAvail().x, 25)))
					{
						ResourcesViewerShader::Open(&material);
					}

					#define TEXID(a) a == nullptr ? 0 : (void*)(uintptr_t)(((TextureOpenGL*)(a))->GetID())
					#define TEXTURE_SELECTOR(a, text) \
						ImGui::PushID(text"##_Textures"); \
						if (ImGui::ImageButton(TEXID(a), TexSize)) \
							ResourcesViewerTexture::Open(&a); \
						ImGui::PopID(); \
						ImGui::SameLine(); \
						ImGui::Text(text);

					ImVec2 TexSize(30, 10);

					// This is a block of texture selectors, which are: ImageButton which activates
					// texture viewer and name of the texture
					TEXTURE_SELECTOR(material.AlbedoMap, "Albedo");
					TEXTURE_SELECTOR(material.NormalMap, "Normal");
					TEXTURE_SELECTOR(material.RoughnessMap, "Roughness");
					TEXTURE_SELECTOR(material.MetallicMap, "Metallic");
					TEXTURE_SELECTOR(material.OcclusionMap, "Occlusion");
					TEXTURE_SELECTOR(material.EmissionMap, "Emission");
					TEXTURE_SELECTOR(material.DetailAlbedoMap, "Detail Albedo");
					TEXTURE_SELECTOR(material.DetailNormalMap, "Detail Normal");

					#undef TEXTURE_SELECTOR
					#undef TEXID
				}

				ImGui::Unindent(10.0f);
				ImGui::Separator();

				CreateNewMaterialDraw(Scn.MaterialsManager);
			}

			ImGui::PopID();
		}
	}

	void CreateNewMaterialDraw(ResourceManager<Material>& MaterialsManager)
	{
		if (CreateNewMaterialOpen && CreateNewMaterialMat != nullptr)
		{
			static std::string Name;

			size_t Selected = 0;
			iVector2 Size(300, 120);

			ImGui::OpenPopup("Create New Material");
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(Size.X, Size.Y));
			if (ImGui::BeginPopupModal("Create New Material", &CreateNewMaterialOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
			{
				ImGui::PushID("CreateNewMaterial");
				bool appearing = ImGui::IsWindowAppearing();

				if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
				    ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape)))
				{
					CreateNewMaterialOpen = false;
					CreateNewMaterialMat = nullptr;
					Name.clear();
				}

				if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
				    ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Enter)))
				{
					Selected = 2;
				}

				ImVec2 Size = ImGui::GetContentRegionAvail();
				if (ImGui::BeginChild("TextField", ImVec2(Size.x, Size.y - 30)))
				{
					if (appearing) ImGui::SetKeyboardFocusHere();
					ImGui::InputText("Name", &Name);
				}
				ImGui::EndChild();

				if (ImGui::BeginChild("Buttons"))
				{
					if (ImGui::Button("Cancel")) Selected = 1;
					ImGui::SameLine();
					if (ImGui::Button("Create")) Selected = 2;
				}
				ImGui::EndChild();

				if (Selected != 0)
				{
					if (Selected == 2)
					{
						SmartPointer<Material> Mat(*CreateNewMaterialMat != nullptr ? new Material(**CreateNewMaterialMat) : new Material());
						if (MaterialsManager.Add(std::move(Mat), Name.c_str()))
						{
							*CreateNewMaterialMat = MaterialsManager[Name.c_str()].Get();
							(*CreateNewMaterialMat)->Name = Name;
						}
					}

					CreateNewMaterialOpen = false;
					Name.clear();
				}
				ImGui::PopID();
			}
			ImGui::EndPopup();
		}
	}

}


