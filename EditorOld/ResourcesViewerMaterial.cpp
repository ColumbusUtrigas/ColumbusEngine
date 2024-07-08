#include <Editor/ResourcesViewerMaterial.h>
#include <Editor/ResourcesViewerFunctions.h>
#include <Editor/FileDialog.h>
#include <Editor/MessageBox.h>
#include <Lib/imgui/imgui.h>
#include <Editor/CommonUI.h>

namespace Columbus::Editor
{

	EditorFileDialog MaterialLoader("./Data/Materials/", {"mat"});
	MessageBox MaterialBruteLoader("One or more materials are already exist",
		"Do you realy want to load it against the already loaded ones?", { 300, 120 });

	bool ResourcesViewerMaterial::Opened = false;
	Material** ResourcesViewerMaterial::Destination = nullptr;
	Material* ResourcesViewerMaterial::Tmp = nullptr;

	void ResourcesViewerMaterial::Draw(Scene& Scn)
	{
		MaterialLoader.MultipleSelect(true);

		static String Find;
		static Material* PopupObject = nullptr;

		if (Opened)
		{
			ImGui::OpenPopup("Materials Viewer");
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(600, 370));
			if (ImGui::BeginPopupModal("Materials Viewer", &Opened, ImGuiWindowFlags_NoResize))
			{
				if (ImGui::IsWindowFocused() &&
				    ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape)))
				{
				    Opened = false;
				}

				auto LoadMore = [&]() { MaterialLoader.Open(); };
				auto Button = [&](const char* Name, void* _) { return ImGui::Button(Name, ImVec2(100, 100)); };
				auto RightClick = [&](Material* PopupObject) {};
				auto DoubleClick = [&]() { Close(); };

				auto Load = [&](const char* Path, Material* S) { return S->Load(Path, Scn.ShadersManager, Scn.TexturesManager); };
				auto Success = [&](const char* _) { MaterialLoader.Close(); };
				auto Failure = [&]() { MaterialLoader.SetMessageBox(&MaterialBruteLoader); MaterialBruteLoader.Open(); };
				auto New = [&]() { return new Material(); };

				ResourceViewerDrawLoadMore("LoadMore_MaterialsViewer", LoadMore);
				ResourceViewerDrawList<Material>("MaterialsList_MaterialsViewer", Tmp, PopupObject, Scn.MaterialsManager, Find, Button, RightClick, DoubleClick);
				ResourceViewerDrawButtons("Buttons_MaterialsViewer", &Destination, Find, [&](){ Close(); }, Opened);
				ResourceViewerLoad<Material>("Load Material", MaterialLoader,
					Scn.MaterialsManager, MaterialBruteLoader,
					Load, Success, Failure, New);

				ImGui::EndPopup();
			}
		}
	}

}


