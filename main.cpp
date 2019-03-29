#include <Scene/Scene.h>
#include <Input/Input.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/WindowOpenGLSDL.h>

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
	scene.Load("Data/2.scene");

	Fireplace = scene.GetGameObject(2);
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
	}

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
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	ImGui::StyleColorsDark();
	
	ImGui_ImplSDL2_InitForOpenGL(window.GetWindowHandle(), window.GetContextHandle());
	ImGui_ImplOpenGL3_Init("#version 130");

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

			window.PollEvent(Event);
		}

		window.Update();
		window.Clear({ 0, 0, 0.75f, 1 });

		input.SetKeyboardFocus(window.HasKeyFocus());
		input.SetMouseFocus(window.HasMouseFocus());

		camera.Perspective(60, window.GetAspect(), 0.1f, 1000);

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

		scene.MainRender.ContextSize = window.GetSize();
		scene.MainRender.SetViewport({0}, window.GetSize());
		scene.Update();
		scene.Render();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window.GetWindowHandle());
		ImGui::NewFrame();

		SDL_ShowCursor(cursor);

		ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
		ImGui::Text("Redraw time: %.1f ms", RedrawTime * 1000);
		ImGui::Text("FPS:         %.1f", 1.0f / RedrawTime);
		ImGui::Separator();
		ImGui::Checkbox("VSync", &VSync);
		ImGui::Checkbox("Bloom", &scene.MainRender.BloomEnable);
		ImGui::SliderFloat("Gamma", &scene.MainRender.Gamma, 0.0, 5.0);
		ImGui::SliderFloat("Exposure", &scene.MainRender.Exposure, 0.0, 5.0);
		ImGui::End();

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.Display();
	}

	delete gDevice;
	delete Sphere;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return 0;
}


