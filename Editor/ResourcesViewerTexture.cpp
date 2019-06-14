#include <Editor/ResourcesViewerTexture.h>
#include <Editor/FileDialog.h>
#include <Editor/MessageBox.h>
#include <Lib/imgui/imgui.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Core/Platform/PlatformFilesystem.h>

namespace Columbus
{

	EditorFileDialog TextureLoader("./Data/Textures/");
	MessageBox BruteLoader("One or more textures are already exist", "Do you realy want to load it against the already loaded ones?", { 300, 120 });

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
		if (Scn != nullptr && Opened)
		{
			ImGui::OpenPopup("Textures Viewer");
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(600, 370));
			if (ImGui::BeginPopupModal("Textures Viewer", &Opened, ImGuiWindowFlags_NoResize))
			{
				if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape))) Opened = false;

				if (ImGui::BeginChild("LoadMore##Textures Viewer", ImVec2(ImGui::GetWindowContentRegionWidth(), 30)))
				{
					if (ImGui::Button("Load More##Texture Viewer_LoadMore", ImVec2(ImGui::GetWindowContentRegionWidth(), 30)))
					{
						TextureLoader.MultipleSelect(true);
						TextureLoader.Open();
					}
				}
				ImGui::EndChild();



				if (ImGui::BeginChild("Textures List##Textures Viewer", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30)))
				{
					uint32 Width = ImGui::GetWindowContentRegionWidth();

					auto TextureButton = [&](Texture* Tex, const char* Name)
					{
						bool Pushed = false;

						// Set color to yellow if this texture is selected
						if (Tex == Tmp)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.73f, 0.60f, 0.15f, 1.00f));
							Pushed = true;
						}

						// Draw an image button with texture and if it clicked, select that texture
						if (ImGui::ImageButton(Tex == nullptr ? 0 : (void*)(uintptr_t)((TextureOpenGL*)(Tex))->GetID(), ImVec2(100, 100)))
							Tmp = Tex;

						// Draw a tooltip with the name of texture
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip(Name);

						// Set color to normal if this texture was selected
						if (Pushed) ImGui::PopStyleColor();

						// Texture buttons packing
						if (Width >= 120)
						{
							Width -= 120;
							ImGui::SameLine();
						} else {
							Width = ImGui::GetWindowContentRegionWidth();
						}
					};

					TextureButton(nullptr, "None");

					for (const auto& Elem : Scn->TexturesManager.ResourcesMap)
					{
						Texture* Tex = Scn->TexturesManager.Resources[Elem.second].Get();
						TextureButton(Tex, Elem.first.c_str());
						
					}
				}
				ImGui::EndChild();


				if (Destination != nullptr)
				{
					if (ImGui::BeginChild("Buttons##Textures Viewer"))
					{
						if (ImGui::Button("Cancel")) Opened = false;
						ImGui::SameLine();
						if (ImGui::Button("Ok")) Close();
					}
					ImGui::EndChild();
				}



				bool IsBruteYes = BruteLoader.GetSelected() == 3;
				if (BruteLoader.GetSelected() != 0) BruteLoader.Reset();

				if (TextureLoader.Draw("Load Texture") || IsBruteYes)
				{
					auto Selected = TextureLoader.GetSelected();
					if (Selected.size() >= 1)
					{
						for (const auto& Elem : Selected)
						{
							auto CodeName = Filesystem::RelativePath(Elem.Path, Filesystem::GetCurrent());

							if (Scn->TexturesManager.IsNameFree(CodeName) || IsBruteYes)
							{
								SmartPointer<Texture> Tex(gDevice->CreateTexture());
								if (Tex->Load(Elem.Path.c_str()))
								{
									Scn->TexturesManager.Add(std::move(Tex), CodeName, IsBruteYes);
									Log::Success("Texture loaded: %s", Elem.Path.c_str());
									TextureLoader.Close();
								}
							} else
							{
								TextureLoader.SetMessageBox(&BruteLoader);
								BruteLoader.Open();
							}
						}
					}
				}

				ImGui::EndPopup();
			}
		}
	}

}


