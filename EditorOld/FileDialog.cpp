#include <Editor/FileDialog.h>
#include <Editor/Icons.h>
#include <Editor/CommonUI.h>
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>
#include <Core/Filesystem.h>
#include <algorithm>

namespace Columbus::Editor
{

	static String Recompose(const std::vector<String>& Decomposed, size_t Index)
	{
		String Result;

		for (size_t i = 0; i <= Index; i++)
		{
			Result += Decomposed[i] + "/";
		}

		return Result.empty() ? Decomposed[0] : Result;
	}

	void EditorFileDialog::_Open()
	{
		
	}

	bool EditorFileDialog::Draw(const String& Name)
	{
		bool res = false;

		if (Opened)
		{
			if (!ImGui::IsPopupOpen(Name.c_str()))
				ImGui::OpenPopup(Name.c_str());

			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSizeConstraints(ImVec2(450, 250), ImVec2(FLT_MAX, FLT_MAX));
			if (ImGui::BeginPopupModal(Name.c_str(), &Opened, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
			{
				if (ImGui::IsWindowFocused() &&
				    ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape)))
				{
					Close();
				}

				// CloseFlag is set via Close() function
				if (CloseFlag)
				{
					ImGui::CloseCurrentPopup();
					Opened = false;
					CloseFlag = false;
				}

				String Absolute = Filesystem::AbsolutePath(Path);
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
						return str_tolower(a.Name) < str_tolower(b.Name);
					};

					auto Finder = [](const auto& a)
					{
						return a.Type == 'f';
					};

					auto Files = Filesystem::Read(Absolute);
					if (!_Filter.empty())
					{
						Files.erase(std::remove_if(Files.begin(), Files.end(), [&](auto& a){
							if (a.Type == 'f')
								return std::find(_Filter.begin(), _Filter.end(), a.Ext.c_str()) == _Filter.end();
							else return false;
						}), Files.end());
					}
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

						String Text;

						switch (Elem.Type)
						{
						case 'd': Text = ICON_FA_FOLDER;        break;
						case 'l': Text = ICON_FA_LINK;          break;
						case 'f': Text = GetFileIcon(Elem.Ext); break;
						}

						Text += " " + Elem.Name;

						bool Contains = std::find(SelectedFiles.begin(), SelectedFiles.end(), Elem) != SelectedFiles.end()
							|| Elem.Name == SaveFile.Name;
						if (ImGui::Selectable(Text.c_str(), Contains, ImGuiSelectableFlags_AllowDoubleClick))
						{
							if (_Type == Type_Save)
							{
								SelectedFiles.clear();
								SaveFile = Elem;
							}

							// click + shift, multiple selection
							if (ImGui::GetIO().KeyShift && _Type == Type_Open && Multiple && !SelectedFiles.empty())
							{
								auto firstIt = std::find(Files.begin(), Files.end(), SelectedFiles.back());
								auto secondIt = std::find(Files.begin(), Files.end(), Elem);

								if (firstIt > secondIt) std::swap(firstIt, secondIt);

								for (; firstIt <= secondIt; ++firstIt)
								{
									// if there is no this elem in selected files
									if (std::find(SelectedFiles.begin(), SelectedFiles.end(), *firstIt) == SelectedFiles.end())
										SelectedFiles.push_back(*firstIt);
								}
							}
							// click + ctrl, multiple selection
							else if (ImGui::GetIO().KeyCtrl && _Type == Type_Open && Multiple)
							{
								if (ImGui::GetIO().KeyCtrl && Contains)
									SelectedFiles.erase(std::remove(SelectedFiles.begin(), SelectedFiles.end(), Elem), SelectedFiles.end());
								else
									SelectedFiles.push_back(Elem);
							}
							// no modifier and/or no multiple selection
							else
							{
								SelectedFiles.clear();
								SelectedFiles.push_back(Elem);
							}

							// double-click on directory
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
					if (_Type == Type_Open)
					{
						ImGui::Text("File:");
						for (const auto& Elem : SelectedFiles)
						{
							ImGui::SameLine();
							ImGui::Text("%s", Elem.Name.c_str());
						}
					}
					else
					{
						ImGui::Text("File:");
						ImGui::SameLine();
						std::string Tmp = SaveFile.Name.c_str();
						ImGui::InputText(("##FileDialog_File_" + Name).c_str(), &Tmp);

						SelectedFiles.clear();

						SaveFile.Name = Tmp.c_str();
						SaveFile.Path = Path + '/' + SaveFile.Name;
						SaveFile.Type = 'f';
					}

					if (ImGui::Button("Cancel")) Close();
					ImGui::SameLine();
					if (ImGui::Button("Ok"))
					{
						res = true;
						if (_Type == Type_Save)
						{
							SelectedFiles.clear();
							SelectedFiles.push_back(SaveFile);
						}
					}
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


