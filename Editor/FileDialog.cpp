#include <Editor/FileDialog.h>
#include <Editor/FontAwesome.h>
#include <Lib/imgui/imgui.h>
#include <Core/Platform/PlatformFilesystem.h>
#include <algorithm>

namespace Columbus
{

	static std::string Recompose(const std::vector<std::string>& Decomposed, size_t Index)
	{
		std::string Result;

		for (size_t i = 0; i <= Index; i++)
		{
			Result += Decomposed[i] + "/";
		}

		return Result.empty() ? Decomposed[0] : Result;
	}

	static const char* GetFileIcon(const std::string& Ext)
	{
		auto e = Ext;
		std::transform(e.begin(), e.end(), e.begin(), ::tolower);
		if (e == "tga" || e == "bmp" || e == "dds" || e == "tif" || e == "tiff" ||
		    e == "jpg" || e == "jpeg" || e == "png") return ICON_FA_FILE_IMAGE_O;
		if (e == "wav" || e == "mp3" || e == "ogg") return ICON_FA_MUSIC;
		if (e == "json" || e == "glsl" || e == "hlsl" || e == "csl") return ICON_FA_CODE;
		if (e == "hdr") return ICON_FA_PICTURE_O;
		if (e == "scene") return ICON_FA_STRIKETHROUGH;
		if (e == "lig") return ICON_FA_LIGHTBULB_O;
		if (e == "mat") return ICON_FA_CIRCLE;
		if (e == "par") return ICON_FA_CERTIFICATE;
		if (e == "cmf" || e == "obj" || e == "dae" || e == "fbx") return ICON_FA_SPACE_SHUTTLE;
		if (e == "ttf") return ICON_FA_FONT;

		return ICON_FA_FILE_O;
	}

	bool EditorFileDialog::Draw(const std::string& Name)
	{
		bool res = false;

		if (Opened)
		{
			ImGui::OpenPopup(Name.c_str());
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSizeConstraints(ImVec2(450, 250), ImVec2(FLT_MAX, FLT_MAX));
			if (ImGui::BeginPopupModal(Name.c_str(), &Opened, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
			{
				if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape))) Close();

				std::string Absolute = Filesystem::AbsolutePath(Path);
				auto Decomposition = Filesystem::Split(Absolute);

				size_t i = 0;
				for (const auto& Elem : Decomposition)
				{
					ImGui::SameLine();
					if (ImGui::Button((Elem + "##" + std::to_string(i)).c_str()))
					{
						Path = Recompose(Decomposition, i);
					}
					i++;
				}

				ImGui::Separator();





				ImVec2 size = ImVec2(ImGui::GetContentRegionMax().x, ImGui::GetContentRegionMax().y - 100.0f);
				if (ImGui::BeginChild((Name + "##FileList").c_str(), size))
				{
					auto Pred = [](const auto& a, const auto& b)
					{
						return (a.Type != 'f' && b.Type == 'f');
					};

					auto Pred2 = [](const auto& a, const auto& b)
					{
						return a.Name < b.Name;
					};

					auto Finder = [](const auto& a)
					{
						return a.Type == 'f';
					};

					auto Files = Filesystem::Read(Absolute + "/");
					std::sort(Files.begin(), Files.end(), Pred);
					auto Limit = std::find_if(Files.begin(), Files.end(), Finder);
					std::sort(Files.begin(), Limit, Pred2);
					std::sort(Limit, Files.end(), Pred2);

					for (const auto& Elem : Files)
					{
						if (Elem.Name == ".") continue;
						if (Elem.Name == "..") continue;
						if (!Hidden)
						{
							if (!Elem.Name.empty())
							{
								if (Elem.Name[0] == '.') continue;
							}
						}

						std::string Text = "";

						switch (Elem.Type)
						{
						case 'd': Text = ICON_FA_FOLDER_O;      break;
						case 'l': Text = ICON_FA_LINK;          break;
						case 'f': Text = GetFileIcon(Elem.Ext); break;
						}

						Text += " " + Elem.Name;

						bool Contains = std::find(SelectedFiles.begin(), SelectedFiles.end(), Elem) != SelectedFiles.end();
						if (ImGui::Selectable(Text.c_str(), Contains, ImGuiSelectableFlags_AllowDoubleClick))
						{
							if (!ImGui::GetIO().KeyCtrl || !Multiple) SelectedFiles.clear();
							if (ImGui::GetIO().KeyCtrl && Contains)
								SelectedFiles.erase(std::remove(SelectedFiles.begin(), SelectedFiles.end(), Elem), SelectedFiles.end());
							else
								SelectedFiles.push_back(Elem);

							if (Elem.Type == 'd' || Elem.Type == 'l')
							{
								if (ImGui::IsMouseDoubleClicked(0))
								{
									Path += Elem.Name + (Elem.Ext.empty() ? "" : '.' + Elem.Ext) + "/";
								}
							}
						}
					}
				}
				ImGui::EndChild();





				if (ImGui::BeginChild((Name + "##Buttons").c_str()))
				{
					ImGui::Text("File:");
					for (const auto& Elem : SelectedFiles)
					{
						ImGui::SameLine();
						ImGui::Text(Elem.Name.c_str());
					}

					if (ImGui::Button("Cancel")) Close();
					ImGui::SameLine();
					if (ImGui::Button("Ok")) res = true;
					ImGui::SameLine();
					ImGui::Checkbox(("Show hidden##Checkbox_" + Name).c_str(), &Hidden);
				}
				ImGui::EndChild();

				if (Message != nullptr)
				{
					Message->Draw();
				}

				ImGui::EndPopup();
			}
		}

		return res;
	}

}


