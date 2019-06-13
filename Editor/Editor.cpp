#include <Editor/Editor.h>
#include <Lib/imgui/imgui.h>
#include <Editor/FileDialog.h>
#include <Editor/FontAwesome.h>
#include <Core/Platform/PlatformFilesystem.h>

namespace Columbus
{

	EditorFileDialog SkyboxLoader("./Data/Skyboxes/");

	void Editor::ApplyDarkTheme()
	{
		ImGuiStyle& Style = ImGui::GetStyle();
		ImGui::StyleColorsDark(&Style);
		Style.FrameRounding = 3.0f;
		Style.WindowRounding = 0.0f;
		Style.ScrollbarRounding = 3.0f;

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		colors[ImGuiCol_ChildBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
		colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
		colors[ImGuiCol_FrameBgActive]          = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
		colors[ImGuiCol_TitleBg]                = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_TitleBgActive]          = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark]              = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_SliderGrab]             = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_Button]                 = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_ButtonHovered]          = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_ButtonActive]           = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_Header]                 = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
		colors[ImGuiCol_HeaderHovered]          = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
		colors[ImGuiCol_HeaderActive]           = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
		colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
		colors[ImGuiCol_SeparatorActive]        = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_ResizeGrip]             = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
		colors[ImGuiCol_Tab]                    = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_TabHovered]             = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_TabActive]              = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram]          = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
		colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
	}

	void Editor::DrawMainMenu(Scene& scene)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open")) scene.Load("Data/3.scene");
				ImGui::MenuItem("Save");
				ImGui::MenuItem("Save As");
				ImGui::MenuItem("Quit");
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Scene", nullptr, &PanelScene.Opened);
				ImGui::MenuItem("Hierarchy", nullptr, &PanelHierarchy.Opened);
				ImGui::MenuItem("Render Settings", nullptr, &PanelRenderSettings.Opened);
				ImGui::MenuItem("Inspector", nullptr, &PanelInspector.Opened);
				ImGui::MenuItem("Profiler", nullptr, &PanelProfiler.Opened);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("GameObject"))
			{
				if (ImGui::MenuItem("Empty")) scene.AddEmpty();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Scene"))
			{
				if (ImGui::MenuItem("Skybox")) SkyboxLoader.Open();//SkyboxWindow = true;
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void Editor::DrawDockSpace(Scene& scene)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowBgAlpha(0.0f);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		if (ImGui::Begin("DockSpace", nullptr, window_flags))
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
			ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

			DrawMainMenu(scene);
		}
		ImGui::End();

		ImGui::PopStyleVar(3);
	}

	Editor::Editor()
	{
		ApplyDarkTheme();

		ImGuiIO& io = ImGui::GetIO();
	 	io.Fonts->AddFontDefault();
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
		io.Fonts->AddFontFromFileTTF("./Data/Font.ttf", 12.0f, &icons_config, icons_ranges);	
		io.Fonts->Build();
	}

	void Editor::Draw(Scene& scene, iVector2& Size, float RedrawTime)
	{
		DrawDockSpace(scene);

		PanelScene.SetFramebufferTexture(scene.MainRender.GetFramebufferTexture());
		PanelHierarchy.SetScene(&scene);
		Size = PanelScene.GetSize();
		PanelRenderSettings.SetRenderer(&scene.MainRender);
		PanelInspector.SetInspectableObject(PanelHierarchy.GetObject());
		PanelProfiler.SetRedrawTime(RedrawTime);

		PanelScene.Draw();
		PanelHierarchy.Draw();
		PanelRenderSettings.Draw();
		PanelInspector.Draw();
		PanelProfiler.Draw();

		if (SkyboxLoader.Draw("Load Skybox"))
		{
			SmartPointer<Texture> Tex(gDevice->CreateTexture());
			if (Tex->Load(SkyboxLoader.GetSelected().Path.c_str()))
			{
				delete scene.Sky;
				scene.Sky = new Skybox(Tex.Get());
				Log::Success("Skybox loaded: %s", SkyboxLoader.GetSelected().Path.c_str());
			}

			SkyboxLoader.Close();
		}
	}

	Editor::~Editor() {}

}


