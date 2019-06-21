#include <Editor/Inspector/PanelInspector.h>
#include <Editor/ResourcesViewerMesh.h>
#include <Editor/Icons.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Lib/imgui/imgui.h>

namespace Columbus
{

	void EditorPanelInspector::DrawComponentMeshRendererEditor(Scene& Scn)
	{
		auto Co = (ComponentMeshRenderer*)Inspectable->GetComponent(Component::Type::MeshRenderer);
		if (Co != nullptr)
		{
			if (ImGui::CollapsingHeader(MESH_ICON" Mesh Renderer##PanelInspector"))
			{
				if (ImGui::Button("Mesh##PanelInspector_MeshRenderer", ImVec2(ImGui::GetContentRegionAvail().x, 25)))
				{
					ResourcesViewerMesh::Open(&Co->GetMesh());
				}
			}
		}
	}

}


