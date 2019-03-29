#include <Scene/Scene.h>
#include <Input/Input.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/WindowOpenGLSDL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>

using namespace Columbus;

#include <Lib/imgui/imgui.h>
#include <Lib/imgui/examples/imgui_impl_opengl3.h>
#include <Lib/imgui/examples/imgui_impl_sdl.h>

#ifdef COLUMBUS_PLATFORM_WINDOWS
	//Hint to the driver to use discrete GPU
	extern "C" 
	{
		//NVIDIA
		__declspec(dllexport) int NvOptimusEnablement = 1;
		//AMD
		__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	}
#endif

GameObject* Fireplace = nullptr;

class FireplaceBright : public Component
{
private:
	float Time = 0.0f;
public:
	virtual void Update(float TimeTick, Transform& Trans) final override
	{
		float Period = 0.5f;
		float Amplitude = 0.1f;
		float Offset = 0.9f;

		float Sin = Math::Sin(Time += TimeTick / Period);
		Sin = Sin < 0.0f ? (0.0f - Sin) : Sin;
		Fireplace->GetMaterial().EmissionStrength = Sin * Amplitude + Offset;
	}
};

void ApplyDarkStyle()
{
	ImGuiStyle& Style = ImGui::GetStyle();
	ImGui::StyleColorsDark(&Style);
	Style.FrameRounding = 3.0f;
	Style.WindowRounding = 0.0f;
	Style.ScrollbarRounding = 3.0f;

	//Style.Colors[ImGuiCol_WindowBg] = ImVec4(25, 25, 25, 255);
	//Style.Colors[ImGuiCol_FrameBg] = ImVec4(48, 53, 61, 138);

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
	colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
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
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

ImVec2 DrawMainMenu(Scene& scene)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open")) scene.Load("Data/2.scene");
			ImGui::MenuItem("Save");
			ImGui::MenuItem("Save As");
			ImGui::MenuItem("Quit");
			ImGui::EndMenu();
		}

		ImVec2 Size = ImGui::GetWindowSize();

		ImGui::EndMainMenuBar();

		return Size;
	}

	return {};
}

void DrawDockSpace(Scene& scene)
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
	ImGui::Begin("DockSpace Demo", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	DrawMainMenu(scene);
	ImGui::End();
}

void DrawDebugWindow(const ImVec2& MainMenuSize, Scene& scene, float RedrawTime)
{
	float Framerate = 1.0f / RedrawTime;

	ImVec4 TextColor(0.0, 1.0, 0.0, 1.0);
	if (Framerate > 0.0f && Framerate < 60.0f)
	{
		InterpolationCurve<Vector3> Curve;
		Curve.AddPoint(Vector3(1, 0, 0), 0.0f);
		Curve.AddPoint(Vector3(0, 1, 0), 60.0f);
		Vector3 Color = Curve.Interpolate(Framerate);
		TextColor = ImVec4(Color.X, Color.Y, Color.Z, 1.0);
	}

	ImGui::SetNextWindowBgAlpha(0.4);
	ImGui::SetNextWindowPos(ImVec2(0, MainMenuSize.y));
	ImGui::Begin("Debug", nullptr, ImVec2(200, 120), -1.0f, ImGuiWindowFlags_NoDecoration);
	ImGui::TextColored(TextColor, "Redraw time: %.1f ms", RedrawTime * 1000);
	ImGui::Text("FPS:         %.1f", 1.0f / RedrawTime);
	ImGui::Separator();
	ImGui::Text("Rendered polygons: %i", scene.MainRender.GetPolygonsRendered());
	ImGui::Text("Rendered objects:  %i", scene.MainRender.GetOpaqueObjectsRendered() + scene.MainRender.GetTransparentObjectsRendered());
	ImGui::End();
}

void DrawMaterialEditor(GameObject* GO)
{
	const char* cull_items[] = { "No", "Front", "Back", "Front and back"};
	const char* depth_items[] = { "Less", "Greater", "LEqual", "GEqual", "Equal", "Not equal", "Never", "Always" };

	ImGui::Combo("Culling", (int*)&GO->GetMaterial().Culling, cull_items, 4);
	ImGui::Combo("Depth test", (int*)&GO->GetMaterial().DepthTesting, depth_items, 8);
	ImGui::Separator();

	ImGui::Checkbox("Depth writing", &GO->GetMaterial().DepthWriting);
	ImGui::Checkbox("Transparent", &GO->GetMaterial().Transparent);
	ImGui::Checkbox("Lighting", &GO->GetMaterial().Lighting);
	ImGui::Separator();
	
	ImGui::DragFloat2("Tiling", (float*)&GO->GetMaterial().Tiling, 0.01f);
	ImGui::DragFloat2("Detail Tiling", (float*)&GO->GetMaterial().DetailTiling, 0.01f);
	ImGui::ColorEdit4("Albedo", (float*)&GO->GetMaterial().Albedo);
	ImGui::SliderFloat("Roughness", &GO->GetMaterial().Roughness, 0.0, 1.0);
	ImGui::SliderFloat("Metallic", &GO->GetMaterial().Metallic, 0.0, 1.0);
	ImGui::DragFloat("Emission Strength", &GO->GetMaterial().EmissionStrength, 0.01f);
}

void DrawTransformEditor(GameObject* GO)
{
	ImGui::DragFloat3("Position", (float*)&GO->transform.Position, 0.1f);
	ImGui::DragFloat3("Rotation", (float*)&GO->transform.Rotation, 0.1f);
	ImGui::DragFloat3("Scale", (float*)&GO->transform.Scale, 0.1f);
}

void DrawEditor(GameObject* GO)
{
	ImGui::Begin("Editor");

	ImGui::Text("%s", GO->Name.c_str());
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Transform"))
	{
		DrawTransformEditor(GO);
		ImGui::Separator();
	}

	if (ImGui::CollapsingHeader("Material"))
	{
		DrawMaterialEditor(GO);
		ImGui::Separator();
	}

	ImGui::End();
}

int main(int argc, char** argv)
{
	WindowOpenGLSDL window({ 640, 480 }, "Columbus Engine", Window::Flags::Resizable);
	gDevice = new DeviceOpenGL();
	Scene scene;
	Input input;
	AudioListener Listener;
	Camera camera;

	camera.Pos = Vector3(10, 10, 0);
	camera.Rot = Vector3(0, 180, 0);

	input.ShowMouseCursor(false);
	input.SetSystemCursor(SystemCursor::Crosshair);

	bool cursor = false;
	//scene.Load("Data/2.scene");

	/*Fireplace = scene.GetGameObject(2);
	Fireplace->AddComponent(new FireplaceBright());

	Mesh* Sphere = gDevice->CreateMesh();

	{
		Model SphereModel;
		SphereModel.Load("Data/Models/Sphere.cmf");
		Sphere->Load(SphereModel);
	}

	GameObject Tests[36];

	float X = -6.0f;
	float Y = 6.0f;

	Transform trans;

	for (int Roughness = 0; Roughness < 6; Roughness++)
	{
		Y = 6.0f;
		X += 2;

		for (int Metallic = 0; Metallic < 6; Metallic++)
		{
			Y -= 2;
			Tests[Roughness * 6 + Metallic].GetMaterial().SetShader(scene.GetGameObject(0)->GetMaterial().GetShader());
			Tests[Roughness * 6 + Metallic].GetMaterial().Albedo = Vector4(1);
			//Tests[Roughness * 6 + Metallic].GetMaterial().Albedo = Vector4(Vector3(0, 1, 0), 0.5);
			//Tests[Roughness * 6 + Metallic].GetMaterial().Transparent = true;
			//Tests[Roughness * 6 + Metallic].GetMaterial().Culling = Material::Cull::No;
			Tests[Roughness * 6 + Metallic].GetMaterial().Roughness = Math::Clamp(Roughness * 0.2f, 0.01f, 1.0f);
			Tests[Roughness * 6 + Metallic].GetMaterial().Metallic = Math::Clamp(Metallic * 0.2f, 0.01f, 1.0f);
			trans.SetPos(Vector3(X, Y, 20));
			Tests[Roughness * 6 + Metallic].SetTransform(trans);
			Tests[Roughness * 6 + Metallic].AddComponent(new ComponentMeshRenderer(Sphere));
			scene.Add(8 + (Roughness * 6 + Metallic), std::move(Tests[Roughness * 6 + Metallic]));
		}
	}*/

	SDL_Event Event;

	float wheel = 0.0f;
	const int CameraSpeed = 5;

	bool VSync = true;
	window.SetVSync(VSync);

	scene.SetCamera(camera);
	scene.SetAudioListener(Listener);
	scene.Audio.Play();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.WantSaveIniSettings = false;
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ApplyDarkStyle();
	
	ImGui_ImplSDL2_InitForOpenGL(window.GetWindowHandle(), window.GetContextHandle());
	ImGui_ImplOpenGL3_Init("#version 130");

	ImVec2 SizeOfRenderWindow;

	while (window.IsOpen())
	{
		float RedrawTime = window.GetRedrawTime();

		input.Update();

		while (SDL_PollEvent(&Event))
		{
			switch (Event.type)
			{
			case SDL_QUIT: window.Close(); break;
			case SDL_KEYDOWN: input.SetKeyDown(Event.key.keysym.scancode); break;
			case SDL_KEYUP:   input.SetKeyUp  (Event.key.keysym.scancode); break;
			case SDL_MOUSEMOTION: input.SetMousePosition({ Event.motion.x, Event.motion.y }); break;
			case SDL_MOUSEBUTTONDOWN: input.SetMouseButtonDown(Event.button.button, Event.button.clicks); break;
			case SDL_MOUSEBUTTONUP:   input.SetMouseButtonUp  (Event.button.button, Event.button.clicks); break;
			case SDL_MOUSEWHEEL: input.SetMouseWheel({ Event.wheel.x, Event.wheel.y }); break;
			case SDL_CONTROLLERAXISMOTION: input.SetGamepadAxis(Event.caxis.axis, (float)(Event.caxis.value) / 32768.0f); break;
			case SDL_CONTROLLERBUTTONDOWN: input.SetGamepadButtonDown(Event.cbutton.button); break;
			case SDL_CONTROLLERBUTTONUP:   input.SetGamepadButtonUp  (Event.cbutton.button); break;
			}

			ImGui_ImplSDL2_ProcessEvent(&Event);
			window.PollEvent(Event);
		}

		window.Update();
		window.Clear({ 0.06f, 0.06f, 0.06f, 1 });

		input.SetKeyboardFocus(window.HasKeyFocus());
		input.SetMouseFocus(window.HasMouseFocus());

		camera.Perspective(60, SizeOfRenderWindow.x / SizeOfRenderWindow.y, 0.1f, 1000);

		VSync = input.GetKeyDown(SDL_SCANCODE_V) ? !VSync : VSync;
		VSync = input.GetGamepadButtonDown(Input::GamepadButton::DPadDown) ? !VSync : VSync;
		window.SetVSync(VSync);

		wheel += input.GetMouseWheel().Y * 5;
		camera.Pos += camera.Direction() * wheel * RedrawTime;
		wheel -= wheel * 3 * RedrawTime;
		if (abs(wheel) <= 0.2) wheel = 0.0f;

		camera.Pos += camera.Direction() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_W);
		camera.Pos -= camera.Direction() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_S);
		camera.Pos -= camera.Right() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_A);
		camera.Pos += camera.Right() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_D);
		camera.Pos -= camera.Up() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_LSHIFT);
		camera.Pos += camera.Up() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_SPACE);

		camera.Rot -= Vector3(0, 0, 120 * RedrawTime) * input.GetKey(SDL_SCANCODE_Q);
		camera.Rot += Vector3(0, 0, 120 * RedrawTime) * input.GetKey(SDL_SCANCODE_E);

		if (input.GetKeyDown(SDL_SCANCODE_ESCAPE))
		{
			cursor = !cursor;
			input.ShowMouseCursor(cursor);
		}

		if (!cursor)
		{
			Vector2 deltaMouse = input.GetMouseMovement();
			camera.Rot += Vector3(deltaMouse.Y, -deltaMouse.X, 0) * 0.3f;
			window.SetMousePosition(window.GetSize() / 2);
			input.SetMousePosition (window.GetSize() / 2);
		}

		float SpeedMultiplier = input.GetGamepadAxis(Input::GamepadAxis::RTrigger) + 1;
		scene.TimeFactor = 1.0f - input.GetGamepadAxis(Input::GamepadAxis::LTrigger);

		camera.Pos += input.GetGamepadStick(Input::GamepadStick::Left).X * camera.Right() * RedrawTime * CameraSpeed * SpeedMultiplier;
		camera.Pos -= input.GetGamepadStick(Input::GamepadStick::Left).Y * camera.Direction() * RedrawTime * CameraSpeed * SpeedMultiplier;
		camera.Rot += Vector3(input.GetGamepadStick(Input::GamepadStick::Right).YX() * Vector2 { 1, -1 } * RedrawTime * 60, 0);

		camera.Rot.Clamp({ -89.9f, -360.0f, -360.0f }, { 89.9f, 360.0f, 360.0f });
		camera.Update();

		Listener.Position = camera.Pos;
		Listener.Right = camera.Right();
		Listener.Up = camera.Up();
		Listener.Forward = camera.Direction();

		scene.MainRender.ContextSize = { (int)SizeOfRenderWindow.x, (int)SizeOfRenderWindow.y };
		scene.MainRender.SetViewport({0}, { (int)SizeOfRenderWindow.x, (int)SizeOfRenderWindow.y });
		scene.Update();
		scene.Render();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window.GetWindowHandle());
		ImGui::NewFrame();

		DrawDockSpace(scene);

		SDL_ShowCursor(cursor);

		ImGui::Begin("Render", nullptr);
		ImGui::Checkbox("VSync", &VSync);

		if (ImGui::CollapsingHeader("Bloom"))
		{
			const char* resolutions[] = { "Full", "Half", "Quad" };

			ImGui::Checkbox("Enable", &scene.MainRender.BloomEnable);
			ImGui::DragFloat("Treshold", &scene.MainRender.BloomTreshold, 0.001f);
			ImGui::DragFloat("Intensity", &scene.MainRender.BloomIntensity, 0.001f);
			ImGui::DragFloat("Radius", &scene.MainRender.BloomRadius, 0.001f);
			ImGui::SliderInt("Iterations", &scene.MainRender.BloomIterations, 1, 4);
			ImGui::Combo("Resolution", (int*)&scene.MainRender.BloomResolution, resolutions, 3);

			ImGui::Separator();
		}

		ImGui::SliderFloat("Gamma", &scene.MainRender.Gamma, 0.0, 5.0);
		ImGui::SliderFloat("Exposure", &scene.MainRender.Exposure, 0.0, 5.0);
		ImGui::End();

		auto GO = scene.GetGameObject("Hercules");
		if (GO) DrawEditor(GO);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		if (ImGui::Begin("Scene"))
		{
			auto Framebuffer = scene.MainRender.GetFramebufferTexture();
			auto Tex = ((TextureOpenGL*)Framebuffer)->GetID();
			auto Size = ImGui::GetWindowSize();
			ImGui::Image((void*)(intptr_t)Tex, ImVec2(Size.x, Size.y - 20), ImVec2(0, 1), ImVec2(1, 0));
			ImGui::PopStyleVar(1);
			ImGui::End();
			SizeOfRenderWindow = Size;
		}

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.Display();
	}

	delete gDevice;
	//delete Sphere;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return 0;
}


