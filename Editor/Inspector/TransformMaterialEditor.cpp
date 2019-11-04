#include <Editor/Inspector/PanelInspector.h>
#include <Editor/ResourcesViewerMaterial.h>
#include <Editor/ResourcesViewerTexture.h>
#include <Editor/ResourcesViewerShader.h>
#include <Editor/Icons.h>
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
		ImGui::DragFloat3("Position##Inspector_TransformEditor", (float*)&Inspectable->transform.Position, 0.1f);
		ImGui::DragFloat3("Rotation##Inspector_TransformEditor", (float*)&Inspectable->transform.Rotation, 0.1f);
		ImGui::DragFloat3("Scale##Inspector_TransformEditor",    (float*)&Inspectable->transform.Scale,    0.1f);
		ImGui::Unindent(10.0f);
	}



	void EditorPanelInspector::DrawMaterialEditor(Scene& Scn)
	{
		const char* CullItems[] = { "No", "Front", "Back", "Front and back"};
		const char* DepthItems[] = { "Less", "Greater", "LEqual", "GEqual", "Equal", "Not equal", "Never", "Always" };

		bool IsStandart = Inspectable->material == Scn.MaterialsManager.Find("Default");
		bool Collapsing = false;

		if (IsStandart)
			Collapsing = ImGui::CollapsingHeader(MATERIAL_ICON" Material (Default)##PanelInspector_Material");
		else
			Collapsing = ImGui::CollapsingHeader(MATERIAL_ICON" Material##PanelInspector_Material");

		if (Collapsing)
		{
			ImGui::Indent(10.0f);

			if (ImGui::Button("Select##Inspector_MaterialEditor"))
			{
				ResourcesViewerMaterial::Open(&Inspectable->material);
			}

			ImGui::SameLine();

			if (ImGui::Button("New##Inspector_MaterialEditor"))
			{
				CreateNewMaterialOpen = true;
				CreateNewMaterialMat = &Inspectable->material;
			}

			if (Inspectable->material != nullptr)
			{
				Material& material = *Inspectable->material;

				ImGui::Spacing();

				ImGui::Combo("Culling##Inspector_MaterialEditor",    (int*)&material.Culling,      CullItems,  4);
				ImGui::Combo("Depth test##Inspector_MaterialEditor", (int*)&material.DepthTesting, DepthItems, 8);
				ImGui::Spacing();

				ImGui::Checkbox("Depth writing##Inspector_MaterialEditor", &material.DepthWriting);
				ImGui::Checkbox("Transparent##Inspector_MaterialEditor",   &material.Transparent);
				ImGui::Checkbox("Lighting##Inspector_MaterialEditor",      &material.Lighting);
				ImGui::Spacing();
				
				ImGui::DragFloat2("Tiling##Inspector_MaterialEditor",        (float*)&material.Tiling,           0.01f);
				ImGui::DragFloat2("Detail Tiling##Inspector_MaterialEditor", (float*)&material.DetailTiling,     0.01f);
				ImGui::ColorEdit4("Albedo##Inspector_MaterialEditor",        (float*)&material.Albedo);
				ImGui::SliderFloat("Roughness##Inspector_MaterialEditor",            &material.Roughness,        0.00f, 1.0f);
				ImGui::SliderFloat("Metallic##Inspector_MaterialEditor",             &material.Metallic,         0.00f, 1.0f);
				ImGui::DragFloat("Emission Strength##Inspector_MaterialEditor",      &material.EmissionStrength, 0.01f);
				ImGui::Spacing();

				if (ImGui::Button("Shader##Inspector_MaterialEditor_Shader", ImVec2(ImGui::GetContentRegionAvail().x, 25)))
				{
					ResourcesViewerShader::Open(&material.ShaderProg);
				}

				#define TEXID(a) a == nullptr ? 0 : (void*)(uintptr_t)(((TextureOpenGL*)(a))->GetID())
				#define TEXTURE_SELECTOR(a, text) \
					ImGui::PushID(text"##Inspector_MaterialEditor_Textures"); \
					if (ImGui::ImageButton(TEXID(a), TexSize)) \
						ResourcesViewerTexture::Open(&a); \
					ImGui::PopID(); \
					ImGui::SameLine(); \
					ImGui::Text(text);

				ImVec2 TexSize(30, 10);

				// This is a block of texture selectors, which are: ImageButton which activates
				// texture viewer and name of the texture
				TEXTURE_SELECTOR(material.AlbedoMap,       "Albedo");
				TEXTURE_SELECTOR(material.NormalMap,       "Normal");
				TEXTURE_SELECTOR(material.RoughnessMap,    "Roughness");
				TEXTURE_SELECTOR(material.MetallicMap,     "Metallic");
				TEXTURE_SELECTOR(material.OcclusionMap,    "Occlusion");
				TEXTURE_SELECTOR(material.EmissionMap,     "Emission");
				TEXTURE_SELECTOR(material.DetailAlbedoMap, "Detail Albedo");
				TEXTURE_SELECTOR(material.DetailNormalMap, "Detail Normal");

				#undef TEXTURE_SELECTOR
				#undef TEXID
			}

			ImGui::Unindent(10.0f);
			ImGui::Separator();

			CreateNewMaterialDraw(Scn.MaterialsManager);
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
				if (ImGui::BeginChild("TextField##CreateNewMaterial", ImVec2(Size.x, Size.y - 30)))
				{
					if (appearing) ImGui::SetKeyboardFocusHere();
					ImGui::InputText("Name", &Name);
				}
				ImGui::EndChild();

				if (ImGui::BeginChild("Buttons##CreateNewMaterial"))
				{
					if (ImGui::Button("Cancel")) Selected = 1;
					ImGui::SameLine();
					if (ImGui::Button("Create")) Selected = 2;
				}
				ImGui::EndChild();
				ImGui::EndPopup();

				if (Selected != 0)
				{
					if (Selected == 2)
					{
						SmartPointer<Material> Mat(*CreateNewMaterialMat != nullptr ? new Material(**CreateNewMaterialMat) : new Material());
						if (MaterialsManager.Add(std::move(Mat), Name.c_str()))
						{
							*CreateNewMaterialMat = MaterialsManager[Name.c_str()].Get();
						}
					}

					CreateNewMaterialOpen = false;
					Name.clear();
				}
			}
		}
	}

}


