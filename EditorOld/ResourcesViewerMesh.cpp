#include <Editor/ResourcesViewerMesh.h>
#include <Editor/ResourcesViewerFunctions.h>
#include <Editor/FileDialog.h>
#include <Editor/MessageBox.h>
#include <Lib/imgui/imgui.h>
#include <Graphics/OpenGL/MeshOpenGL.h>
#include <Graphics/Device.h>

namespace Columbus::Editor
{

	EditorFileDialog MeshLoader("./Data/Meshes/", {"cmf", "obj"});
	MessageBox MeshBruteLoader("One or more meshes are already exist",
		"Do you realy want to load it against the already loaded ones?", { 300, 120 });

	bool ResourcesViewerMesh::Opened = false;
	Mesh** ResourcesViewerMesh::Destination = nullptr;
	Mesh* ResourcesViewerMesh::Tmp = nullptr;

	void ResourcesViewerMesh::Draw(Scene* Scn)
	{
		MeshLoader.MultipleSelect(true);

		static String Find;
		static Mesh* PopupObject = nullptr;

		if (Scn != nullptr && Opened)
		{
			ImGui::OpenPopup("Meshes Viewer");
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(600, 370));
			if (ImGui::BeginPopupModal("Meshes Viewer", &Opened, ImGuiWindowFlags_NoResize))
			{
				if (ImGui::IsWindowFocused() &&
				    ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape)))
				{
					Opened = false;
				}

				auto LoadMore = [&]() { MeshLoader.Open(); };
				auto Button = [&](const char* Name, void* _) { return ImGui::Button(Name, ImVec2(100, 100)); };
				auto RightClick = [&](Mesh* PopupObject) {};
				auto DoubleClick = [&]() { Close(); };

				auto Load = [&](const char* Path, Mesh* S) { return S->Load(Path); };
				auto Success = [&](const char* _) { MeshLoader.Close(); };
				auto Failure = [&]() { MeshLoader.SetMessageBox(&MeshBruteLoader); MeshBruteLoader.Open(); };
				auto New = [&]() { return gDevice->CreateMesh(); };

				ResourceViewerDrawLoadMore("LoadMore_MeshesViewer", LoadMore);
				ResourceViewerDrawList<Mesh>("MeshesList_MeshesViewer", Tmp, PopupObject, Scn->MeshesManager, Find, Button, RightClick, DoubleClick);
				ResourceViewerDrawButtons("Buttons_MeshesViewer", Destination, Find, [&](){ Close(); }, Opened);
				ResourceViewerLoad<Mesh>("Load Mesh", MeshLoader,
					Scn->MeshesManager, MeshBruteLoader,
					Load, Success, Failure, New);

				ImGui::EndPopup();
			}
		}
	}

}


