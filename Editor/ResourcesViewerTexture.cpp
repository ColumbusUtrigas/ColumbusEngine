#include <Editor/ResourcesViewerTexture.h>
#include <Editor/ResourcesViewerFunctions.h>
#include <Editor/FileDialog.h>
#include <Editor/MessageBox.h>
#include <Lib/imgui/imgui.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Core/Platform/PlatformFilesystem.h>
#include <Graphics/Device.h>

namespace Columbus
{

	EditorFileDialog TextureLoader("./Data/Textures/");
	MessageBox BruteLoader("One or more textures are already exist",
		"Do you realy want to load it against the already loaded ones?", { 300, 120 });

	bool ResourcesViewerTexture::Opened = false;
	Texture** ResourcesViewerTexture::Destination = nullptr;
	Texture* ResourcesViewerTexture::Tmp = nullptr;

	//
	//
	// TODO: Sort textures by name
	// TODO: Reload textures button
	//
	//

	void ResourcesViewerTexture::Draw(Scene* Scn)
	{
		TextureLoader.MultipleSelect(true);

		static std::string Find;
		static Texture* PopupObject = nullptr;

		if (Scn != nullptr && Opened)
		{
			ImGui::OpenPopup("Textures Viewer");
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(600, 370));
			if (ImGui::BeginPopupModal("Textures Viewer", &Opened, ImGuiWindowFlags_NoResize))
			{
				if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape))) Opened = false;

				#define TEXID(a) (void*)(uintptr_t)((TextureOpenGL*)(a))->GetID()

				auto LoadMore = [&]() { TextureLoader.Open(); };
				auto Button = [&](const char* Name, Texture* Tex)
				{
					bool Result = ImGui::ImageButton(Tex == nullptr ? 0 : TEXID(Tex), ImVec2(100, 100));
					return Result;
				};
				auto RightClick = [&](Texture* PopupObject)
				{
					
				};
				auto DoubleClick = [&]() { Close(); };

				#undef TEXID

				auto Load = [&](const char* Path, Texture* Tex) { return Tex->Load(Path); };
				auto Success = [&](const char* Path) { Log::Success("Texture loaded: %s", Path); TextureLoader.Close(); };
				auto Failure = [&]() { TextureLoader.SetMessageBox(&BruteLoader); BruteLoader.Open(); };
				auto New = [&]() { return gDevice->CreateTexture(); };

				ResourceViewerDrawLoadMore("LoadMore_TexturesViewer", LoadMore);
				ResourceViewerDrawList<Texture>("TexturesList_ShadersViewer", Tmp, PopupObject, Scn->TexturesManager, Find, Button, RightClick, DoubleClick);
				ResourceViewerDrawButtons("Buttons_TexturesViewer", Destination, Find, [&](){ Close(); }, Opened);
				ResourceViewerLoad<Texture>("Load Texture", TextureLoader,
					Scn->TexturesManager, BruteLoader,
					Load, Success, Failure, New);

				ImGui::EndPopup();
			}
		}
	}

}


