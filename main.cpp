#include <Scene/Scene.h>
#include <Input/Input.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/WindowOpenGLSDL.h>

#include <Editor/Editor.h>
#include <Profiling/Profiling.h>

#include <Input/EventSystem.h>

//#include <Graphics/Vulkan/InstanceVulkan.h>

using namespace Columbus;

#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_opengl3.h>
#include <imgui/examples/imgui_impl_sdl.h>
#include <ImGuizmo/ImGuizmo.h>

#ifdef PLATFORM_WINDOWS
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
//#define FLAPPY_BIRD_GAME

#ifdef FLAPPY_BIRD_GAME
class BirdComponent : public Component
{
private:
	float jump = 2;
	float gravity = 4.8;
	float speed = jump;
	float xspeed = 0.5;
	Camera& _camera;
	Input& input;
	Scene& scene;
public:
	BirdComponent(Camera& camera, Input& input, Scene& scene) : _camera(camera), input(input), scene(scene) {}

	void Update(float DeltaTime) final override
	{
		if (input.GetKeyDown(SDL_SCANCODE_SPACE))
		{
			speed = jump;
		}

		float angle = -speed * 30;
		gameObject->transform.Position.X += xspeed * DeltaTime;
		gameObject->transform.Position.Y += speed * DeltaTime;
		gameObject->transform.Rotation.X = -90;
		gameObject->transform.Rotation.Y = Math::Clamp<float>(angle, -80, 80) + 90;
		speed -= gravity * DeltaTime;
		_camera.Pos.X += xspeed * DeltaTime;
	}

	~BirdComponent() final override {}
};

class ColumnsComponent : public Component
{
private:
	std::vector<GameObject*> columns;
	int n = 10;
	ShaderProgram* shader;
	Mesh* mesh;
	Scene& scene;

	void generate(int i)
	{
		columns[i]->transform.Position = Random::Range<float>(-1, -0.5);
	}
public:
	ColumnsComponent(ShaderProgram* shader, Mesh* mesh, Scene& scene) : shader(shader), mesh(mesh), scene(scene) {}
	void OnComponentAdd() final override
	{
		for (int i = 0; i < n; i++)
		{
			GameObject obj;
			obj.Name = String::from(i);
			obj.material = new Material();
			obj.material->SetShader(shader);
			obj.material->EmissionMap = gDevice->GetDefaultTextures()->White.get();
			obj.transform.Rotation = { -90, 0, 0 };
			obj.transform.Scale = { 0.1, 0.1, 1 };
			obj.AddComponent(new ComponentMeshRenderer(mesh));
			scene.Add(std::move(obj));
			columns.push_back(scene.Objects.Find(String::from(i)));
			generate(i);
		}
	}

	void Update(float DeltaTime) final override
	{
	}
	~ColumnsComponent() final override {}
};

int main(int argc, char** argv)
{
	WindowOpenGLSDL window({ 640, 480 }, "Flappy Bird (Columbus Engine)", Window::Flags::Resizable);
	gDevice = new DeviceOpenGL();
	gDevice->Initialize();
	Scene scene;
	Input input;
	EventSystem eventSystem;
	Camera camera;
	Renderer render;

	bool run = true;

	eventSystem.QuitFunction = [&](const Event&) { run = false; };
	eventSystem.InputFunction = [&](const Event& E) { input.PollEvent(E); };
	eventSystem.WindowFunction = [&](const Event& E) { window.PollEvent(E); };
	eventSystem.RawFunction = [&](void* Raw) {};

	window.SetVSync(true);

	ShaderProgram* shader = gDevice->CreateShaderProgram();
	shader->Load("Data/Shaders/Standart.csl");
	shader->Compile();

	Texture* tex = gDevice->CreateTexture();
	Texture::Flags flags;
	flags.Filtering = Texture::Filter::Point;
	tex->Load("Data/FlappyBird/bird.png");
	tex->SetFlags(flags);

	Mesh* plane = gDevice->CreateMesh();
	plane->Load("Data/Meshes/Plane.obj");

	{
		GameObject obj;
		obj.Name = "Bird";
		obj.material = new Material();
		obj.material->AlbedoMap = tex;
		obj.material->EmissionMap = tex;
		obj.material->SetShader(shader);
		obj.transform.Scale = { 0.07 };
		scene.Add(std::move(obj));
	}
	GameObject* bird = scene.Objects.Find("Bird");
	bird->AddComponent(new ComponentMeshRenderer(plane));
	bird->AddComponent(new BirdComponent(camera, input, scene));

	//for (int i = 0; i < 10; i++)
	{
		GameObject obj;
		//obj.Name = String::from(i);
		obj.Name = "Columns";
		//obj.material = new Material();
		//obj.material->SetShader(shader);
		//obj.material->EmissionMap = gDevice->GetDefaultTextures()->White.get();
		//obj.AddComponent(new ComponentMeshRenderer(plane));
		//obj.transform.Position = { i * 0.5f + 1, -1, -0.1 };
		//obj.transform.Rotation = { -90, 0, 0 };
		//obj.transform.Scale = { 0.1, 0.1, 1 };
		scene.Add(std::move(obj));
		scene.Objects.Find("Columns")->AddComponent(new ColumnsComponent(shader, plane, scene));
	}

	while (run && window.IsOpen())
	{
		float deltaTime = window.GetRedrawTime();
		input.Update();
		eventSystem.Update();
		input.SetKeyboardFocus(window.HasKeyFocus());
		input.SetMouseFocus(window.HasMouseFocus());
		window.Clear({ 1, 0, 0, 1 });

		scene.Update();

		camera.Ortho(-window.GetAspect(), window.GetAspect(), -1, 1, 0.001, 10);
		camera.Update();

		render.ContextSize = window.GetSize();
		render.SetDeltaTime(deltaTime);
		render.SetViewport({ 0 }, window.GetSize());
		render.SetMainCamera(camera);
		render.SetSky(scene.Sky);
		render.SetScene(&scene);
		render.SetRenderList(&scene.Objects.Resources);
		render.SetIsEditor(false);
		render.Render();

		window.Display();
	}

	gDevice->Shutdown();
	delete gDevice;

	return 0;
}
#endif

#ifndef FLAPPY_BIRD_GAME
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
#endif //FLAPPY_BIRD_GAME


