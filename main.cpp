#if 0
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
#include <Scene/ComponentParticleSystem.h>
class BirdComponent : public Component
{
public:
	const float Jump = 2;
	const float Gravity = 4.8;
	const float XSpeed = 0.5;
	float Speed = Jump;
public:
	void Respawn()
	{
		gameObject->transform.Position = {};
		gameObject->transform.Rotation = {};
		Speed = Jump;
	}
	Component* Clone() const final override
	{
		return new BirdComponent();
	}

	void Update(float DeltaTime) final override {}
	~BirdComponent() final override {}
};

class FlappyBirdGameComponent : public Component
{
private:
	std::vector<GameObject*> columns;
	int n = 10;
	Scene& scene;
	Camera& camera;
	Input& input;
	BirdComponent* bird;
	GameObject* particles;

	void generate(int i, int add)
	{
		columns[i]->transform.Position.X += add;
		columns[i]->transform.Position.Y = Random::Range<float>(-1, -0.5);
	}

	void respawn()
	{
		camera.Pos.X = 0;
		bird->Respawn();
		for (int i = 0; i < n; i++)
		{
			columns[i]->transform.Position.X = 0;
			generate(i, i + 1);
		}
	}
public:
	FlappyBirdGameComponent(Scene& scene, Camera& camera, Input& input) :
		scene(scene), camera(camera), input(input) {}

	Component* Clone() const final override
	{
		return new FlappyBirdGameComponent(scene, camera, input);
	}

	void OnComponentAdd() final override
	{
		ShaderProgram* shader = gDevice->CreateShaderProgram();
		shader->Load("Data/Shaders/Standart.csl");
		shader->Compile();

		Mesh* plane = gDevice->CreateMesh();
		plane->Load("Data/Meshes/Plane.obj");

		Texture* tex = gDevice->CreateTexture();
		tex->Load("Data/FlappyBird/bird.png");

		Texture* pipe = gDevice->CreateTexture();
		pipe->Load("Data/FlappyBird/pipe.png");

		// create bird
		GameObject bobj;
		bobj.material = new Material();
		bobj.Name = "Bird";
		bobj.material->SetShader(shader);
		bobj.material->AlbedoMap = tex;
		bobj.material->EmissionMap = tex;
		bobj.material->EmissionStrength = 1;
		bobj.material->SetShader(shader);
		bobj.transform.Scale = { 0.07 };
		bobj.AddComponent(new ComponentMeshRenderer(plane));
		scene.Add(std::move(bobj));

		bird = new BirdComponent();
		scene.Objects.Find("Bird")->AddComponent(bird);

		// generate columns
		for (int i = 0; i < n; i++)
		{
			GameObject obj;
			obj.Name = String::from(i);
			obj.material = new Material();
			obj.material->SetShader(shader);
			obj.material->AlbedoMap = pipe;
			obj.material->EmissionMap = pipe;
			obj.material->EmissionStrength = 0.7;
			obj.transform.Position.X = i;
			obj.transform.Rotation = { -90, 0, 0 };
			obj.transform.Scale = { 0.1, 0.1, 1 };
			obj.AddComponent(new ComponentMeshRenderer(plane));
			scene.Add(std::move(obj));
			columns.push_back(scene.Objects.Find(String::from(i)));
			generate(i, 1);
		}

		auto parShader = gDevice->CreateShaderProgram();
		parShader->Load("Data/Shaders/Particles.csl");
		parShader->Compile();

		// create particles
		GameObject par;
		par.Name = "Particles";
		par.material = new Material();
		par.material->SetShader(parShader);
		scene.Add(std::move(par));
		ParticleEmitterCPU ps;
		ps.MaxParticles = 100;
		ps.EmitRate = 40;
		ps.ModuleSize.Mode = ParticleModuleSize::UpdateMode::Initial;
		ps.ModuleSize.Min = { 0.005f };
		ps.ModuleSize.Max = { 0.005f };
		ps.ModuleVelocity.Min = { -1, -1, 0 };
		ps.ModuleVelocity.Max = { -0.5, 1, 0 };
		particles = scene.Objects.Find("Particles");
		particles->AddComponent(new ComponentParticleSystem(std::move(ps)));
	}

	void Update(float DeltaTime) final override
	{
		// update columns
		for (int i = 0; i < n; i++)
		{
			auto& tr = columns[i]->transform;
			if (tr.Position.X - camera.Pos.X + 0.2 < -3)
			{
				generate(i, n);
			}
		}

		// update bird
		if (input.GetKeyDown(SDL_SCANCODE_SPACE))
		{
			bird->Speed = bird->Jump;
		}
		float angle = -bird->Speed * 30;
		bird->gameObject->transform.Position.X += bird->XSpeed * DeltaTime;
		bird->gameObject->transform.Position.Y += bird->Speed * DeltaTime;
		bird->gameObject->transform.Rotation.X = -90;
		bird->gameObject->transform.Rotation.Y = Math::Clamp<float>(angle, -80, 80) + 90;
		bird->Speed -= bird->Gravity * DeltaTime;
		camera.Pos.X += bird->XSpeed * DeltaTime;
		particles->transform.Position = bird->gameObject->transform.Position;

		if (bird->gameObject->transform.Position.Y > 1 || bird->gameObject->transform.Position.Y < -1)
		{
			respawn();
		}

		auto intersect = [](Vector2 a, Vector2 b)->bool
		{
			return (a.X > b.X && a.X < b.Y) || (a.Y > b.X && a.Y < b.Y) || (b.X > a.X&& b.X < a.Y) || (b.Y > a.X&& b.Y < a.Y);
		};

		// collision detection with columns
		for (const auto& column : columns)
		{
			const auto& ctp = column->transform.Position;
			const auto& cts = column->transform.Scale.XZY();
			const auto& btp = bird->gameObject->transform.Position;
			const auto& bts = bird->gameObject->transform.Scale.XZY();
			Vector2 cx = { ctp.X - cts.X, ctp.X + cts.X };
			Vector2 bx = { btp.X - bts.X, btp.X + bts.X };
			Vector2 cy = { ctp.Y - cts.Y, ctp.Y + cts.Y };
			Vector2 by = { btp.Y - bts.Y, btp.Y + bts.Y };

			if (intersect(cx, bx) && intersect(cy, by))
			{
				respawn();
			}
		}
	}
	~FlappyBirdGameComponent() final override {}
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

	window.SetVSync(false);

	{
		GameObject obj;
		obj.Name = "Game";
		scene.Add(std::move(obj));
		scene.Objects.Find("Game")->AddComponent(new FlappyBirdGameComponent(scene, camera, input));
	}
	scene.SetCamera(camera);

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

		render.Tonemapping = TonemappingType::ACES;
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

	camera.Pos = Vector3(0, 10, 0);
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
#else
	MainRender.Tonemapping = TonemappingType::RomBinDaHouse;
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
		gDevice->BeginMarker("IMGUI");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window.GetWindowHandle());
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		Editor.Draw(scene, MainRender, SizeOfRenderWindow, RedrawTime);

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		gDevice->EndMarker();
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

#endif
