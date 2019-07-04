#include <Editor/Inspector/PanelInspector.h>
#include <Editor/ResourcesViewerTexture.h>
#include <Editor/ResourcesViewerShader.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Lib/imgui/imgui.h>

namespace Columbus
{

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

		ImGui::Indent(10.0f);

		Material& material = Scn.Materials[Inspectable->materialID];

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

		ImGui::Unindent(10.0f);
	}

}


