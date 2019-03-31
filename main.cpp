#include <Scene/Scene.h>
#include <Input/Input.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/WindowOpenGLSDL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>

#include <Editor/Editor.h>
#include <Profiling/Profiling.h>

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

	input.ShowMouseCursor(true);
	input.SetSystemCursor(SystemCursor::Crosshair);

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
	
	ImGui_ImplSDL2_InitForOpenGL(window.GetWindowHandle(), window.GetContextHandle());
	ImGui_ImplOpenGL3_Init("#version 130");

	Editor Editor;

	iVector2 SizeOfRenderWindow(1);

	SDL_Event Event;

	while (window.IsOpen())
	{
		ResetProfiling();
		PROFILE_CPU(ProfileModule::CPU);
		float RedrawTime = window.GetRedrawTime();

		input.Update();

		bool Exit = false;

		while (SDL_PollEvent(&Event))
		{
			switch (Event.type)
			{
			case SDL_QUIT: window.Close(); Exit = true; break;
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

		if (Exit) break;

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

		input.ShowMouseCursor(true);
		if (input.GetMouseButton(SDL_BUTTON_RIGHT).State && Editor.PanelScene.IsHover())
		{
			input.ShowMouseCursor(false);

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

			iVector2 PosOfRenderWindow = Editor.PanelScene.GetPosition();
			iVector2 MousePos = PosOfRenderWindow + (SizeOfRenderWindow / 2);

			window.SetMousePosition(MousePos);
			input.SetMousePosition (MousePos);
		}

		camera.Rot.Clamp({ -89.9f, -360.0f, -360.0f }, { 89.9f, 360.0f, 360.0f });
		camera.Update();

		Listener.Position = camera.Pos;
		Listener.Right = camera.Right();
		Listener.Up = camera.Up();
		Listener.Forward = camera.Direction();

		scene.MainRender.ContextSize = SizeOfRenderWindow;
		scene.MainRender.SetViewport({0}, SizeOfRenderWindow);
		scene.Update();
		scene.Render();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window.GetWindowHandle());
		ImGui::NewFrame();

		SDL_ShowCursor(input.IsMouseCursorShowed());

		Editor.Draw(scene, SizeOfRenderWindow, RedrawTime);

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.Display();
	}

	delete gDevice;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_Quit();

	return 0;
}


