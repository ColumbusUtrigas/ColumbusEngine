#pragma once

#include <Editor/Inspector/ComponentEditor.h>
#include <Editor/ResourcesViewerMesh.h>
#include <Scene/ComponentMeshRenderer.h>
#include <imgui/imgui.h>

namespace Columbus::Editor
{

	class ComponentEditorMeshRenderer : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentMeshRenderer, ComponentEditorMeshRenderer);
	public:
		void OnInspectorGUI() final override
		{
			auto comp = static_cast<ComponentMeshRenderer*>(Target);
			if (ImGui::Button("Mesh", ImVec2(ImGui::GetContentRegionAvail().x, 25)))
			{
				ResourcesViewerMesh::Open(&comp->GetMesh());
			}
		}
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentMeshRenderer, ComponentEditorMeshRenderer);

}
