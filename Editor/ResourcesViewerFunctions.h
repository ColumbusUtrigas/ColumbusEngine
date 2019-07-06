#pragma once

#include <Core/Types.h>
#include <Scene/Scene.h>
#include <Editor/FileDialog.h>
#include <Editor/Icons.h>
#include <Core/Platform/PlatformFilesystem.h>
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>
#include <functional>
#include <string>

namespace Columbus
{

	static void ResourceViewerDrawLoadMore(const char* Name, std::function<void()> LoadMore);

	template <typename T>
	static void ResourceViewerDrawSelectable(const char* Name, T* Object, T*& Tmp, uint32& Width,
		std::string& SelectedPopup,
		std::function<bool(const char*, T*)> Button, std::function<void()> DoubleClick);

	template <typename T>
	static void ResourceViewerDrawList(const char* Name, T*& Tmp, T*& PopupObject,
		ResourceManager<T>& Manager, const std::string& Find,
		std::function<bool(const char*, T*)> Button, std::function<void(T*)> RightClick,
		std::function<void()> DoubleClick);

	static void ResourceViewerDrawButtons(const char* Name, const void* Dst, std::string& Find, std::function<void()> Close, bool& Opened);

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
			if (ImGui::Button((std::string("Load More##") + Name).c_str(), ImVec2(ImGui::GetWindowContentRegionWidth(), 30)))
			{
				LoadMore();
			}
		}
		ImGui::EndChild();
	}

	template <typename T>
	void ResourceViewerDrawSelectable(const char* Name, T* Object, T*& Tmp, uint32& Width,
		std::string& SelectedPopup,
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
			Tmp = Object;

		// Draw a tooltip with the name of object
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(Name);

			if (ImGui::IsMouseClicked(1))
				SelectedPopup = Name;

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
		} else {
			Width = (uint32)ImGui::GetWindowContentRegionWidth();
		}
	}

	template <typename T>
	void ResourceViewerDrawList(const char* Name, T*& Tmp, T*& PopupObject,
		ResourceManager<T>& Manager, const std::string& Find,
		std::function<bool(const char*, T*)> Button, std::function<void(T*)> RightClick,
		std::function<void()> DoubleClick)
	{
		std::string PopupStr;

		if (ImGui::BeginChild(Name, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30)))
		{
			uint32 Width = (uint32)ImGui::GetWindowContentRegionWidth();

			ResourceViewerDrawSelectable<T>("None", nullptr, Tmp, Width, PopupStr, Button, DoubleClick);

			std::string MFind = Find;
			std::string MName;

			std::transform(MFind.begin(), MFind.end(), MFind.begin(), ::tolower);

			bool PopupSet = false;

			for (const auto& Elem : Manager.Resources)
			{
				MName = Elem.first;
				std::transform(MName.begin(), MName.end(), MName.begin(), ::tolower);
				
				if (MName.find(MFind) != std::string::npos)
				{
					T* Object = Elem.second.Get();
					ResourceViewerDrawSelectable<T>(Manager.Names[Elem.first].c_str(), Object, Tmp, Width, PopupStr, Button, DoubleClick);

					if (!PopupStr.empty() && !PopupSet)
					{
						PopupObject = Object;
						PopupSet = true;
					}
				}
			}
		}
		ImGui::EndChild();

		if (!PopupStr.empty() && PopupStr != "None")
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

	void ResourceViewerDrawButtons(const char* Name, const void* Dst, std::string& Find, std::function<void()> Close, bool& Opened)
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

			ImGui::InputText("Find##ResourceViewerButtons", &Find);
		}
		ImGui::EndChild();
	}

	template <typename T>
	static void ResourceViewerLoadNew(T* New, ResourceManager<T>& Manager, const char* Path, bool Force,
		std::function<bool(const char*, T*)> Load, std::function<void(const char*)> Success,
		std::function<void()> Failure)
	{
		auto CodeName = Filesystem::RelativePath(Path, Filesystem::GetCurrent());

		if (Manager.IsNameFree(CodeName) || Force)
		{
			SmartPointer<T> Object(New);
			if (Load(Path, New))
			{
				Manager.Add(std::move(Object), CodeName, Force);
				Success(CodeName.c_str());
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


