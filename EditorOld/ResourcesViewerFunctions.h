#pragma once

#include <Core/Types.h>
#include <Core/String.h>
#include <Core/Filesystem.h>
#include <Editor/FileDialog.h>
#include <Editor/Icons.h>
#include <Editor/CommonUI.h>
#include <Lib/imgui/imgui.h>
#include <functional>

namespace Columbus::Editor
{

	static void ResourceViewerDrawLoadMore(const char* Name, std::function<void()> LoadMore);

	template <typename T>
	static void ResourceViewerDrawSelectable(const char* Name, T* Object, T*& Tmp, uint32& Width,
		String& SelectedPopup, bool& IsRight,
		std::function<bool(const char*, T*)> Button, std::function<void()> DoubleClick);

	template <typename T>
	static void ResourceViewerDrawList(const char* Name, T*& Tmp, T*& PopupObject,
		ResourceManager<T>& Manager, const String& Find,
		std::function<bool(const char*, T*)> Button, std::function<void(T*)> RightClick,
		std::function<void()> DoubleClick);

	static void ResourceViewerDrawButtons(const char* Name, const void* Dst, String& Find, std::function<void()> Close, bool& Opened);

	template <typename T>
	static void ResourceViewerLoadNew(T* New, ResourceManager<T>& Manager, const char* Path, bool Force,
		std::function<bool(const char*, T*)> Load, std::function<void(const char*)> Success,
		std::function<void()> Failure);

	template <typename T>
	static void ResourceViewerLoad(const char* Name, EditorFileDialog& Loader,
		ResourceManager<T>& Manager, MessageBox& BruteLoader,
		std::function<bool(const char*, T*)> Load, std::function<void(const char*)> Success,
		std::function<void()> Failure, std::function<T*()> New);





	void ResourceViewerDrawLoadMore(const char* Name, std::function<void()> LoadMore)
	{
		if (ImGui::BeginChild(Name, ImVec2(ImGui::GetWindowContentRegionWidth(), 30)))
		{
			if (ImGui::Button((String("Load More##") + Name).c_str(), ImVec2(ImGui::GetWindowContentRegionWidth(), 30)))
			{
				LoadMore();
			}
		}
		ImGui::EndChild();
	}

	template <typename T>
	void ResourceViewerDrawSelectable(const char* Name, T* Object, T*& Tmp, uint32& Width,
		String& SelectedPopup, bool& IsRight,
		std::function<bool(const char*, T*)> Button, std::function<void()> DoubleClick)
	{
		bool Pushed = false;

		// Set color to yellow if this object is selected
		if (Object == Tmp)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.73f, 0.60f, 0.15f, 1.00f));
			Pushed = true;
		}

		if (Button(Name, Object))
		{
			Tmp = Object;
		}

		if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
		{
			Tmp = Object;
			SelectedPopup = Name;
			IsRight = false;
			DoubleClick();
		}

		// Draw a tooltip with the name of object
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", Name);

			if (ImGui::IsMouseClicked(0))
			{
				SelectedPopup = Name;
				IsRight = false;
			}

			if (ImGui::IsMouseClicked(1))
			{
				SelectedPopup = Name;
				IsRight = true;
			}

			if (ImGui::IsMouseDoubleClicked(0))
				DoubleClick();
		}

		// Set color to normal if this object was selected
		if (Pushed) ImGui::PopStyleColor();

		// Texture buttons packing
		if (Width >= 120)
		{
			Width -= 120;
			ImGui::SameLine();
		} else
		{
			Width = (uint32)ImGui::GetWindowContentRegionWidth();
		}
	}

	template <typename T>
	void ResourceViewerDrawList(const char* Name, T*& Tmp, T*& PopupObject,
		ResourceManager<T>& Manager, const String& Find,
		std::function<bool(const char*, T*)> Button, std::function<void(T*)> RightClick,
		std::function<void()> DoubleClick)
	{
		String PopupStr;
		bool IsRight = false;

		if (ImGui::BeginChild(Name, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30)))
		{
			uint32 Width = (uint32)ImGui::GetWindowContentRegionWidth();

			ResourceViewerDrawSelectable<T>("None", nullptr, Tmp, Width, PopupStr, IsRight, Button, DoubleClick);

			String MFind = str_tolower(Find);
			String MName;

			bool PopupSet = false;

			for (const auto& Elem : Manager.Resources)
			{
				MName = str_tolower(Manager.Names[Elem.first]);
				
				if (MName.find(MFind) != String::npos)
				{
					T* Object = Elem.second.Get();
					ResourceViewerDrawSelectable<T>(Manager.Names[Elem.first].c_str(), Object, Tmp, Width, PopupStr, IsRight, Button, DoubleClick);

					if (!PopupStr.empty() && !PopupSet)
					{
						PopupObject = Object;
						PopupSet = true;
					}
				}
			}
		}
		ImGui::EndChild();

		if (!PopupStr.empty() && PopupStr != "None" && IsRight)
		{
			ImGui::OpenPopup("##ResourceViewer_Popup");
		}

		if (ImGui::IsPopupOpen("##ResourceViewer_Popup"))
		{
			if (ImGui::BeginPopup("##ResourceViewer_Popup"))
			{
				RightClick(PopupObject);
				ImGui::EndPopup();
			}
		}
	}

	void ResourceViewerDrawButtons(const char* Name, const void* Dst, String& Find, std::function<void()> Close, bool& Opened)
	{
		if (ImGui::BeginChild(Name))
		{
			if (Dst != nullptr)
			{
				if (ImGui::Button("Cancel")) Opened = false;
				ImGui::SameLine();
				if (ImGui::Button("Ok")) Close();
				ImGui::SameLine();
			}

			char Tmp[1024];
			memcpy(Tmp, Find.data(), Find.size() + 1);
			ImGui::InputText("Find##ResourceViewerButtons", Tmp, 1024);
			Find = Tmp;
		}
		ImGui::EndChild();
	}

	template <typename T>
	static void ResourceViewerLoadNew(T* New, ResourceManager<T>& Manager, const char* Path, bool Force,
		std::function<bool(const char*, T*)> Load, std::function<void(const char*)> Success,
		std::function<void()> Failure)
	{
		auto pt = String(Path);
		auto name = pt.substr(pt.rfind('/') + 1);

		if (Manager.IsNameFree(name) || Force)
		{
			SmartPointer<T> Object(New);
			if (Load(Path, New))
			{
				Manager.Add(std::move(Object), name, Force);
				Success(name.c_str());
			}
		} else
		{
			Failure();
		}
	}

	template <typename T>
	void ResourceViewerLoad(const char* Name, EditorFileDialog& Loader,
		ResourceManager<T>& Manager, MessageBox& BruteLoader,
		std::function<bool(const char*, T*)> Load, std::function<void(const char*)> Success,
		std::function<void()> Failure, std::function<T*()> New)
	{
		bool Force = BruteLoader.GetSelected() == 3;
		if (BruteLoader.GetSelected() != 0) BruteLoader.Reset();

		if (Loader.Draw(Name) || Force)
		{
			auto Selected = Loader.GetSelected();
			if (Selected.size() >= 1)
			{
				for (const auto& Elem : Selected)
				{
					ResourceViewerLoadNew<T>(New(), Manager, Elem.Path.c_str(), Force, Load, Success, Failure);
				}
			}
		}
	}

}


