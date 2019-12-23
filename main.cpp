#include <Scene/Scene.h>
#include <Input/Input.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/WindowOpenGLSDL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>

#include <Editor/Editor.h>
#include <Profiling/Profiling.h>

#include <Input/EventSystem.h>

//#include <Graphics/Vulkan/InstanceVulkan.h>

using namespace Columbus;

#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_opengl3.h>
#include <imgui/examples/imgui_impl_sdl.h>
#include <ImGuizmo/ImGuizmo.h>

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

#define COLUMBUS_EDITOR

int main(int argc, char** argv)
{
	WindowOpenGLSDL window({ 640, 480 }, "Columbus Engine", Window::Flags::Resizable);
	gDevice = new DeviceOpenGL();
	gDevice->Initialize();
	Scene scene;
	Input input;
	EventSystem eventSystem;
	AudioListener Listener;
	Camera camera;
	Renderer MainRender;

	// Veeeeery experimental
	//GAPIVulkan vk;
	//vk._test();

	camera.Pos = Vector3(10, 10, 0);
	camera.Rot = Vector3(0, 180, 0);

	input.ShowMouseCursor(true);
	input.SetSystemCursor(SystemCursor::Arrow);

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
	//io.WantSaveIniSettings = false;
	io.IniFilename = "interface.ini";
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	
#ifdef COLUMBUS_EDITOR
	ImGui_ImplSDL2_InitForOpenGL(window.GetWindowHandle(), window.GetContextHandle());
	ImGui_ImplOpenGL3_Init("#version 130");
#endif

	Editor Editor;

	iVector2 SizeOfRenderWindow(1);

	bool Running = true;

	eventSystem.QuitFunction = [&](const Event&) { Running = false; };
	eventSystem.InputFunction = [&](const Event& E) { input.PollEvent(E); };
	eventSystem.WindowFunction = [&](const Event& E) { window.PollEvent(E); };
	eventSystem.RawFunction = [&](void* Raw) { ImGui_ImplSDL2_ProcessEvent((SDL_Event*)Raw); };

	window.SetVSync(true);

	bool wasLooking = false;

	scene.Load("Data/Shadows.scene");

	while (Running && window.IsOpen())
	{
		input.Update();
		eventSystem.Update();
#ifndef COLUMBUS_EDITOR
		SizeOfRenderWindow = window.GetSize();
#endif

		if (!Running) break;

		ResetProfiling();
		PROFILE_CPU(ProfileModule::CPU);
		float RedrawTime = window.GetRedrawTime();
		MainRender.SetDeltaTime(RedrawTime);

		window.Clear({ 0.06f, 0.06f, 0.06f, 1 });

		input.SetKeyboardFocus(window.HasKeyFocus());
		input.SetMouseFocus(window.HasMouseFocus());

		camera.Perspective(60, (float)SizeOfRenderWindow.X / (float)SizeOfRenderWindow.Y, 0.1f, 1000);

		VSync = input.GetKeyDown(SDL_SCANCODE_V) ? !VSync : VSync;
		window.SetVSync(VSync);

		if (Editor.PanelScene.IsHover())
		{
			wheel += input.GetMouseWheel().Y * 5;
		}
		
		camera.Pos += camera.Direction() * wheel * RedrawTime;
		wheel -= wheel * 3 * RedrawTime;
		if (abs(wheel) <= 0.2) wheel = 0.0f;

		if (input.GetMouseButton(SDL_BUTTON_RIGHT).State)
		{
#ifdef COLUMBUS_EDITOR
			if (Editor.PanelScene.IsHover())
			{
				wasLooking = true;
			}
#else`
			wasLooking = true;
#endif

			if (wasLooking)
			{
				camera.Pos += camera.Direction() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_W);
				camera.Pos -= camera.Direction() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_S);
				camera.Pos -= camera.Right() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_A);
				camera.Pos += camera.Right() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_D);
				camera.Pos -= camera.Up() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_LSHIFT);
				camera.Pos += camera.Up() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_SPACE);

				camera.Rot -= Vector3(0, 0, 120 * RedrawTime) * input.GetKey(SDL_SCANCODE_Q);
				camera.Rot += Vector3(0, 0, 120 * RedrawTime) * input.GetKey(SDL_SCANCODE_E);

				Vector2 deltaMouse = input.GetMouseMovement();
				camera.Rot += Vector3(deltaMouse.Y, -deltaMouse.X, 0) * 0.3f;
			}
		} else
		{
			wasLooking = false;
		}

		camera.Rot.Clamp({ -89.9f, -360.0f, -360.0f }, { 89.9f, 360.0f, 360.0f });
		camera.Update();

		Listener.Position = camera.Pos;
		Listener.Right = camera.Right();
		Listener.Up = camera.Up();
		Listener.Forward = camera.Direction();

#ifdef COLUMBUS_EDITOR
		MainRender.SetIsEditor(true);
#else
		MainRender.SetIsEditor(false);
#endif

		MainRender.ContextSize = SizeOfRenderWindow;
		MainRender.SetViewport({0}, SizeOfRenderWindow);
		scene.Update();
		MainRender.SetMainCamera(camera);
		MainRender.SetSky(scene.Sky);
		MainRender.SetScene(&scene);
		MainRender.SetRenderList(&scene.Objects.Resources);
		MainRender.Render();

#ifdef COLUMBUS_EDITOR
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window.GetWindowHandle());
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		Editor.Draw(scene, MainRender, SizeOfRenderWindow, RedrawTime);

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

		window.Display();
	}

#ifdef COLUMBUS_EDITOR
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
#endif

	gDevice->Shutdown();
	delete gDevice;

	return 0;
}


