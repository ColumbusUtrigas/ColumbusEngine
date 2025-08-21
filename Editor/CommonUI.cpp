#include <Editor/CommonUI.h>
#include <Editor/Icons.h>
#include <Core/Reflection.h>
#include <Core/Filesystem.h>
#include <Math/Quaternion.h>
#include <Scene/Project.h>
#include <Graphics/World.h>
#include <imgui_internal.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>
#include <Lib/nativefiledialog/src/include/nfd.h>

#include <filesystem>


namespace Columbus::Editor
{
	_CommonUISettings CommonUISettings;

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Custom UI specialisations for reflected types

	bool ImGui_EditVector2(char* Object, const Reflection::Field& Field, int Depth)
	{
		return ImGui::InputFloat2(Field.Name, (float*)Object);
	}

	bool ImGui_EditVector3(char* Object, const Reflection::Field& Field, int Depth)
	{
		if (strstr(Field.Meta, "Colour"))
		{
			ImGuiColorEditFlags Flags = ImGuiColorEditFlags_Float;
			if (strstr(Field.Meta, "HDR"))
				Flags |= ImGuiColorEditFlags_HDR;

			return ImGui::ColorEdit3(Field.Name, (float*)Object, Flags);
		}

		return ImGui::InputFloat3(Field.Name, (float*)Object);
	}

	bool ImGui_EditVector4(char* Object, const Reflection::Field& Field, int Depth)
	{
		if (strstr(Field.Meta, "Colour"))
		{
			ImGuiColorEditFlags Flags = ImGuiColorEditFlags_Float;
			if (strstr(Field.Meta, "HDR"))
				Flags |= ImGuiColorEditFlags_HDR;

			return ImGui::ColorEdit4(Field.Name, (float*)Object, Flags);
		}

		return ImGui::InputFloat4(Field.Name, (float*)Object);
	}

	bool ImGui_EditQuaternion(char* Object, const Reflection::Field& Field, int Depth)
	{
		Quaternion* Quat = (Quaternion*)Object;
		Vector3 Euler = Quat->Euler();
		bool Changed = ImGui::InputFloat3(Field.Name, (float*)&Euler);
		if (Changed)
		{
			*Quat = Quaternion(Euler);
		}
		return Changed;
	}

	CREFLECT_STRUCT_CUSTOM_UI(Vector2, ImGui_EditVector2);
	CREFLECT_STRUCT_CUSTOM_UI(Vector3, ImGui_EditVector3);
	CREFLECT_STRUCT_CUSTOM_UI(Vector4, ImGui_EditVector4);
	CREFLECT_STRUCT_CUSTOM_UI(Quaternion, ImGui_EditQuaternion);

	// Custom UI specialisations for reflected types
	///////////////////////////////////////////////////////////////////////////////////////////////


	void ApplyDarkTheme()
	{
		ImGuiStyle& Style = ImGui::GetStyle();
		ImGui::StyleColorsDark(&Style);
		Style.FrameRounding = 3.0f;
		Style.WindowRounding = 0.0f;
		Style.ScrollbarRounding = 3.0f;

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.75f, 0.75f, 0.75f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
	}

	void FlagButton(const char* name, bool& enabled, const char* tooltip)
	{
		if (ImGui::Button(name, enabled))
			enabled = !enabled;

		if (tooltip != NULL)
			ShowTooltipDelayed(CommonUISettings.TooltipDelay, tooltip);
	}

	bool ToolButton(const char* label, int* v, int v_button, const ImVec2& size, const char* tooltip)
	{
		const bool pressed = ImGui::Button(label, *v == v_button, false, size);
		if (pressed)
			*v = v_button;

		if (tooltip != NULL)
			ShowTooltipDelayed(CommonUISettings.TooltipDelay, tooltip);

		return pressed;
	}

	void ToggleButton(const char* label, bool* v, const char* tooltip)
	{
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		float height = ImGui::GetFrameHeight();
		float width = height * 1.55f;
		float radius = height * 0.50f;

		ImGui::InvisibleButton(label, ImVec2(width, height));
		if (ImGui::IsItemClicked())
			*v = !*v;

		if (tooltip != NULL)
			ShowTooltipDelayed(CommonUISettings.TooltipDelay, tooltip);

		float t = *v ? 1.0f : 0.0f;

		ImGuiContext& g = *GImGui;
		float ANIM_SPEED = 0.08f;
		if (g.LastActiveId == g.CurrentWindow->GetID(label))// && g.LastActiveIdTimer < ANIM_SPEED)
		{
			float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
			t = *v ? (t_anim) : (1.0f - t_anim);
		}

		ImU32 col_bg;
		if (ImGui::IsItemHovered())
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
		else
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
		draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
	}

	void ShowTooltipDelayed(float delay, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > delay)
		{
			ImGui::SetTooltipV(fmt, args);
		}

		va_end(args);
	}

	const char* GetFileIcon(const std::string& ext)
	{
		std::string e = str_tolower(ext);

		if (Filesystem::IsImage(ext)) return ICON_FA_FILE_IMAGE;
		if (e == "wav" || e == "mp3" || e == "ogg") return ICON_FA_MUSIC;
		if (e == "json" || e == "glsl" || e == "hlsl" || e == "csl") return ICON_FA_CODE;
		if (e == "hdr" || e == "exr") return ICON_FA_FILE_IMAGE;
		if (e == "scene") return ICON_FA_STRIKETHROUGH;
		if (e == "lig") return LIGHT_ICON;
		if (e == "mat") return MATERIAL_ICON;
		if (e == "par") return PARTICLES_ICON;
		if (e == "cmf" || e == "obj" || e == "dae" || e == "fbx") return MESH_ICON;
		if (e == "ttf") return ICON_FA_FONT;

		return ICON_FA_FILE;
	}

	struct InternalModalWindow
	{
		std::string Name;
		std::function<bool()> DrawCallback;
	};

	static std::vector<InternalModalWindow> ModalWindowsList;

	void ShowModalWindow(const char* Name, std::function<bool()> DrawCallback)
	{
		ModalWindowsList.push_back(InternalModalWindow
		{
			.Name = Name,
			.DrawCallback = DrawCallback,
		});
	}

	void TickAllModalWindows()
	{
		for (int i = 0; i < (int)ModalWindowsList.size(); i++)
		{
			InternalModalWindow& Window = ModalWindowsList[i];
			bool Open = true;
			bool WasClosed = false;

			ImVec2 Size(500, 250);

			ImGui::OpenPopup(Window.Name.c_str());
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(Size.x, Size.y));
			if (ImGui::BeginPopupModal(Window.Name.c_str(), &Open, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
			{
				if (Window.DrawCallback())
				{
					ModalWindowsList.erase(ModalWindowsList.begin() + i--);
					WasClosed = true;
				}

				ImGui::EndPopup();
			}

			if (!Open && !WasClosed)
			{
				ModalWindowsList.erase(ModalWindowsList.begin() + i--);
			}
		}
	}

	void ShowMessageBox(const char* Name, const char* Text, MessageBoxCallbacks Callbacks)
	{
		std::string TextStr = Text;

		ShowModalWindow(Name, [TextStr, Callbacks]() -> bool
		{
			ImVec2 Size = ImGui::GetContentRegionAvail();
			if (ImGui::BeginChild("TextField##MessageBox", ImVec2(Size.x, Size.y - 30)))
			{
				ImGui::TextWrapped("%s", TextStr.c_str());
			}
			ImGui::EndChild();

			bool Result = false;

			if (ImGui::BeginChild("Buttons##MessageBox"))
			{
				if (ImGui::Button("Cancel"))
				{
					if (Callbacks.OnCancel)
						Callbacks.OnCancel();
					Result = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("No"))
				{
					if (Callbacks.OnNo)
						Callbacks.OnNo();
					Result = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Yes"))
				{
					if (Callbacks.OnYes)
						Callbacks.OnYes();
					Result = true;
				}
			}
			ImGui::EndChild();

			return Result;
		});
	}



	///////////////////////////////////////////////////////////////////////////////////////////////
	// Editing of reflected objects

	bool Reflection_EditObjectField(char* Object, const Reflection::Field& Field, int Depth)
	{
		char* FieldData = Object + Field.Offset;

		switch (Field.Type)
		{
		case Reflection::FieldType::Bool:
			return ImGui::Checkbox(Field.Name, (bool*)FieldData);
			break;
		case Reflection::FieldType::Int:
			return ImGui::InputInt(Field.Name, (int*)FieldData);
			break;
		case Reflection::FieldType::Float:
			return ImGui::InputFloat(Field.Name, (float*)FieldData);
			break;
		case Reflection::FieldType::String:
		{
			if (strstr(Field.Meta, "Picker"))
			{
				ImGui::LabelText(Field.Name, "%s", ((std::string*)FieldData)->c_str());
				ImGui::SameLine();
				if (ImGui::Button("..."))
				{
					char* path = nullptr;
					if (NFD_OpenDialog("gltf,clvl", nullptr, &path) == NFD_OKAY)
					{
						((std::string*)FieldData)->assign(path);
						return true;
					}

					return false;
				}
			}
			else if (strstr(Field.Meta, "Noedit"))
			{
				ImGui::LabelText(Field.Name, "%s", ((std::string*)FieldData)->c_str());
			}
			else
			{
				return ImGui::InputText(Field.Name, (std::string*)FieldData);
			}
		}
		break;

		case Reflection::FieldType::Array:
		{
			std::vector<char>* ArrayData = (std::vector<char>*)FieldData;
			Reflection::ArrayData* Array = Field.Array;

			u32 ElementSize = Array->ElementField.Size;
			u32 NumElements = (u32)ArrayData->size() / ElementSize;

			bool Result = false;
			if (ImGui::CollapsingHeader(Field.Name))
			{
				if (NumElements == 0)
				{
					ImGui::Text("Empty array");
				}

				i32 ElementToRemove = -1;

				ImGui::Indent(Depth * 5.0f);
				u32 Offset = 0;
				for (u32 i = 0; i < NumElements; i++)
				{
					ImGui::PushID(i);
					Result |= Reflection_EditObjectField(ArrayData->data() + Offset, Array->ElementField, Depth + 1);
					ImGui::SameLine();

					if (ImGui::Button("Remove"))
					{
						ElementToRemove = i;
					}

					ImGui::PopID();

					Offset += ElementSize;
				}
				ImGui::Unindent(Depth * 5.0f);

				if (ImGui::Button("Add Element"))
				{
					Array->NewElement(FieldData);
					Result = true;
				}

				if (ElementToRemove > -1)
				{
					Array->DeleteElement(FieldData, ElementToRemove);
					Result = true;
				}
			}
			return Result;
		}
		break;

		case Reflection::FieldType::Enum:
		{
			int EnumValue = *(int*)FieldData;

			const Reflection::Enum* Enum = Field.Enum;
			const Reflection::EnumField* EnumField = Enum->FindFieldByValue(EnumValue);
			int Idx = EnumField->Index;

			bool Result = ImGui::Combo(Field.Name, &Idx, [](void* data, int idx, const char** out_text) -> bool
				{
					*out_text = ((Reflection::Enum*)data)->Fields[idx].Name;
					return true;
				}, (void*)Enum, Enum->Fields.size());

			*((int*)(FieldData)) = Enum->Fields[Idx].Value;

			return Result;
		}
		break;


		case Reflection::FieldType::Struct:
		{
			const Reflection::Struct* Struct = Field.Struct;
			float Indentation = Depth * 5.0f;
			bool Result = false;

			if (Struct->CustomUI)
			{
				Result = Struct->CustomUI(FieldData, Field, Depth + 1);
			}
			else
			{
				if (ImGui::CollapsingHeader(Field.Name))
				{
					for (const auto& SField : Struct->Fields)
					{
						ImGui::Indent(Indentation);
						ImGui::PushID(SField.Name);
						Result |= Reflection_EditObjectField(FieldData, SField, Depth + 1);
						ImGui::PopID();
						ImGui::Unindent(Indentation);
					}
				}
			}

			return Result;
		}
		break;

		case Reflection::FieldType::AssetRef:
		{
			struct AssetRefBase
			{
				std::string Path;
				// templated reference is not present here
			};

			ImGui::LabelText(Field.Name, "%s", ((AssetRefBase*)FieldData)->Path.c_str());
			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				char* path = nullptr;

				const char* AssetExts = "";
				const Reflection::Struct* AssetType = Reflection::FindStructByGuid(Field.Typeguid);

				if (AssetSystem::Get().AssetExtensions.contains(AssetType))
				{
					AssetExts = AssetSystem::Get().AssetExtensions[AssetType];
				}

				if (NFD_OpenDialog(AssetExts, nullptr, &path) == NFD_OKAY)
				{
					if (!AssetSystem::Get().IsPathInBakedFolder(path))
					{
						const auto& AssetBasePath = AssetSystem::Get().DataPath;

						char ErrorBuf[4096]{ 0 };
						snprintf(ErrorBuf, 4096, "Cannot choose asset (%s) - any referenced asset has to be under project's Data folder (%s)", path, AssetBasePath.c_str());

						Editor::ShowMessageBox("Asset Reference Error", ErrorBuf, {});
						Log::Error(ErrorBuf);
						return false;
					}

					((AssetRefBase*)FieldData)->Path = AssetSystem::Get().MakePathRelativeToBakedFolder(path);
					return true;
				}

				return false;
			}
			break;
		}

		case Reflection::FieldType::ThingRef:
		{
			ThingRef<AThing>* Ref = reinterpret_cast<ThingRef<AThing>*>(FieldData);

			// Get all things in the world
			std::vector<AThing*> things;
			if (GCurrentProject && GCurrentProject->World)
			{
				for (AThing* thing : GCurrentProject->World->AllThings)
				{
					if (Reflection::HasParentType(thing->GetTypeVirtual(), Reflection::FindStructByGuid(Field.Typeguid)))
					{
						things.push_back(thing);
					}
				}
			}

			// Build display names
			std::vector<std::string> thingNames;
			int currentIdx = -1;
			for (size_t i = 0; i < things.size(); ++i)
			{
				const auto& thing = things[i];
				thingNames.push_back(thing->Name + " (" + std::to_string((u64)thing->Guid) + ")");
				if (Ref->Thing == thing)
					currentIdx = static_cast<int>(i);
			}

			// Add "None" option
			thingNames.insert(thingNames.begin(), "None");
			if (currentIdx == -1)
				currentIdx = 0;
			else
				currentIdx += 1;

			bool changed = false;
			if (ImGui::Combo(Field.Name, &currentIdx,
				[](void* data, int idx, const char** out_text) -> bool {
					auto& names = *static_cast<std::vector<std::string>*>(data);
					*out_text = names[idx].c_str();
					return true;
				}, &thingNames, static_cast<int>(thingNames.size())))
			{
				if (currentIdx == 0)
				{
					Ref->Thing = nullptr;
					Ref->Guid = 0;
				}
				else
				{
					Ref->Thing = things[currentIdx - 1];
					Ref->Guid = Ref->Thing->Guid;
				}
				changed = true;
			}
			return changed;
		}

		default:
			ImGui::LabelText("Unsupported Type", "%s %s", Field.Typename, Field.Name);
			break;
		}

		return false;
	}

	bool Reflection_EditStruct(char* Object, const Reflection::Struct* Struct)
	{
		ImGui::LabelText("Struct Guid", "%s", Struct->Guid);
		ImGui::LabelText("Struct Version", "%i", Struct->Version);

		bool AnyFieldChanged = false;

		for (const auto& Field : Struct->Fields)
		{
			AnyFieldChanged |= Reflection_EditObjectField(Object, Field);
		}

		if (AnyFieldChanged && Struct->ChangeNotify)
		{
			Struct->ChangeNotify(Object);
		}

		return AnyFieldChanged;
	}

	// Editing of reflected objects
	///////////////////////////////////////////////////////////////////////////////////////////////
}

namespace ImGui
{
	void SetNextWindowPosCenter(ImGuiCond cond)
	{
		auto DisplaySize = ImGui::GetIO().DisplaySize;
		auto Center = ImVec2(DisplaySize.x / 2, DisplaySize.y / 2);
		ImGui::SetNextWindowPos(Center, cond, ImVec2(0.5f, 0.5f));
	}

	bool TreeNodeSized(const char* label, ImVec2 size, ImGuiTreeNodeFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		float LineSizeY = window->DC.CurrLineSize.y;
		float LineBaseOffset = window->DC.CurrLineTextBaseOffset;

		ImVec2 Size = size;

		if (Size.x == 0) Size.x = Columbus::Editor::CommonUISettings.TreeNodeSize.x;
		if (Size.y == 0) Size.y = Columbus::Editor::CommonUISettings.TreeNodeSize.y;

		if (Size.x < 0) Size.x += GetContentRegionAvail().x;
		if (Size.y < 0) Size.y += GetContentRegionAvail().y;

		bool Open = false;
		if (ImGui::BeginChild("TreeNode", Size))
		{
			window = GetCurrentWindow();
			window->DC.CurrLineSize.y = LineSizeY;
			window->DC.CurrLineTextBaseOffset = LineBaseOffset;

			Open = ImGui::TreeNodeEx(label, flags);
		}
		ImGui::EndChild();

		return Open;
	}

	bool Button(const char* label, bool activated, bool hover_col_change, const ImVec2& size)
	{
		auto active = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
		auto passive = ImGui::GetStyle().Colors[ImGuiCol_Button];
		auto current = activated ? active : passive;

		if (!hover_col_change)
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, current);

		ImGui::PushStyleColor(ImGuiCol_Button, current);
		bool result = ImGui::Button(label, size);
		ImGui::PopStyleColor();

		if (!hover_col_change)
			ImGui::PopStyleColor();

		return result;
	}

	/*void Image(Columbus::Texture* texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
	{
		switch (Columbus::gDevice->GetCurrentAPI())
		{
			case Columbus::GraphicsAPI::None: break;
			case Columbus::GraphicsAPI::OpenGL:
			{
				ImTextureID id = nullptr;
				if (texture)
					id = (ImTextureID)static_cast<Columbus::TextureOpenGL*>(texture)->GetID();

				Image(id, size, uv0, uv1, tint_col, border_col);
				break;
			}
		}
	}*/
}
