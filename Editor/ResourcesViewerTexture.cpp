#include <Editor/ResourcesViewerTexture.h>
#include <Editor/ResourcesViewerFunctions.h>
#include <Editor/FileDialog.h>
#include <Editor/MessageBox.h>
#include <Lib/imgui/imgui.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Graphics/Device.h>

namespace Columbus::Editor
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

		static String Find;
		static Texture* PopupObject = nullptr;
		static Texture::Flags Flags;

		if (Scn != nullptr && Opened)
		{
			ImGui::OpenPopup("Textures Viewer");
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			//ImGui::SetNextWindowSize(ImVec2(600, 370));
			ImGui::SetNextWindowSize(ImVec2(750, 370));
			if (ImGui::BeginPopupModal("Textures Viewer", &Opened, ImGuiWindowFlags_NoResize))
			{
				if (ImGui::IsWindowFocused() &&
				    ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape)))
				{
					Opened = false;
				}

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

				if (ImGui::BeginChild("TexturesViewer_Main", ImVec2(600, ImGui::GetContentRegionAvail().y)))
				{
					ResourceViewerDrawLoadMore("LoadMore_TexturesViewer", LoadMore);
					ResourceViewerDrawList<Texture>("TexturesList_ShadersViewer", Tmp, PopupObject, Scn->TexturesManager, Find, Button, RightClick, DoubleClick);
					ResourceViewerDrawButtons("Buttons_TexturesViewer", Destination, Find, [&](){ Close(); }, Opened);
					ResourceViewerLoad<Texture>("Load Texture", TextureLoader,
						Scn->TexturesManager, BruteLoader,
						Load, Success, Failure, New);
				}
				ImGui::EndChild();

				ImGui::SameLine();
				if (ImGui::BeginChild("TexturesViewer_Options"))
				{
					if (PopupObject != nullptr)
					{
						const char* Filterings[] = { "Point", "Linear", "Bilinear", "Trilinear" };
						const char* Anisotropies[] = { "1", "2", "4", "8", "16" };
						const char* Wraps[] = { "Clamp", "Repeat", "MirroredRepeat" };

						ImGui::Text(Scn->TexturesManager.Find(PopupObject).c_str());
						Flags = PopupObject->GetFlags();
						ImGui::Combo("Filtering##TextureViewer_Options", (int*)&Flags.Filtering, Filterings, 4);
						ImGui::Combo("Anisotropy##TextureViewer_Options", (int*)&Flags.AnisotropyFilter, Anisotropies, 5);
						ImGui::Combo("Wrap##TextureViewer_Options", (int*)&Flags.Wrapping, Wraps, 3);
						PopupObject->SetFlags(Flags);
					}
				}
				ImGui::EndChild();

				ImGui::EndPopup();
			}
		}
	}

}


