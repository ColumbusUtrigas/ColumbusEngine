#include <Scene/Scene.h>
#include <Input/Input.h>
#include <Editor/Editor.h>
#include <Profiling/Profiling.h>
#include <Core/Game.h>

#include <Graphics/OpenGL/DeviceOpenGL.h>
//#include <Graphics/Vulkan/InstanceVulkan.h>

using namespace Columbus;

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <imgui/examples/imgui_impl_opengl3.h>
#include <imgui/examples/imgui_impl_win32.h>
#include <Windows.h>

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

HWND hwnd;
HDC hdc;
HGLRC hrc;

Game game;
Input input;

bool quit = false;
const char* class_name = "Columbus Engine Window Class";
iVector2 wnd_size = { 640, 480 };

IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wparam, lparam);

	Event e;
	memset(&e, 0, sizeof(e));

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		wnd_size = { LOWORD(lparam), HIWORD(lparam) };
		break;
	case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
	{
		int button = 0;
		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
		if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
		if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
		if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) ? 3 : 4; }
		if (!ImGui::IsAnyMouseDown() && ::GetCapture() == NULL)
			::SetCapture(hwnd);

		e.Type = Event::Type_MouseButton;
		e.MouseButton.Clicks = 1;
		e.MouseButton.Code = button;
		e.MouseButton.Pressed = true;
		break;
	}
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	{
		int button = 0;
		if (msg == WM_LBUTTONUP) { button = 0; }
		if (msg == WM_RBUTTONUP) { button = 1; }
		if (msg == WM_MBUTTONUP) { button = 2; }
		if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) ? 3 : 4; }
		e.Type = Event::Type_MouseButton;
		e.MouseButton.Clicks = 1;
		e.MouseButton.Code = button;
		e.MouseButton.Pressed = false;
		if (!ImGui::IsAnyMouseDown() && ::GetCapture() == hwnd)
			::ReleaseCapture();
		break;
	}
	case WM_MOUSEMOVE:
		e.Mouse = { LOWORD(lparam), HIWORD(lparam) };
		e.Type = Event::Type_Mouse;
		break;
	case WM_MOUSEWHEEL:
		e.MouseWheel = { 0, GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA };
		e.Type = Event::Type_MouseWheel;
		break;
	case WM_MOUSEHWHEEL:
		e.MouseWheel = { HIWORD(lparam), 0 };
		e.Type = Event::Type_MouseWheel;
		break;
	default:
		return DefWindowProc(hWnd, msg, wparam, lparam);
	}

	input.PollEvent(e);

	return 0;
}

void InitWindowAndContext()
{
	HINSTANCE hInstance = GetModuleHandle(0);

	WNDCLASS wc = { 0 };
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = class_name;
	if (!RegisterClass(&wc))
		COLUMBUS_ASSERT(false);

	hwnd = CreateWindow(class_name, "Columbus Engine", WS_OVERLAPPEDWINDOW, 6, 12, wnd_size.X, wnd_size.Y, 0, 0, hInstance, 0);

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	hdc = GetDC(hwnd);

	int fmt = ChoosePixelFormat(hdc, &pfd);
	COLUMBUS_ASSERT(fmt != 0);
	if (SetPixelFormat(hdc, fmt, &pfd) != TRUE) COLUMBUS_ASSERT(false);

	hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);

	printf("%s\n", glGetString(GL_VERSION));

	ShowWindow(hwnd, SW_SHOWNORMAL);

	glewInit();
	OpenGL::Init();
	gDevice = new DeviceOpenGL();
	gDevice->Initialize();
}

void ShutdownWindowAndContext()
{
	delete gDevice;
	wglDeleteContext(hrc);
	ReleaseDC(hwnd, hdc);
	DestroyWindow(hwnd);
}

void InitGUI()
{
#ifdef COLUMBUS_EDITOR
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
#endif
}

void InitRenderGUI(HWND hwnd)
{
#ifdef COLUMBUS_EDITOR
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.WantSaveIniSettings = false;
	io.IniFilename = "interface.ini";
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplOpenGL3_Init("#version 130");
#endif
}

void RenderBeginGUI()
{
#ifdef COLUMBUS_EDITOR
	gDevice->BeginMarker("IMGUI");
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
#endif
}

void RenderEndGUI()
{
#ifdef COLUMBUS_EDITOR
	auto& io = ImGui::GetIO();
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	gDevice->EndMarker();
#endif
}

void ShutdownGUI()
{
#ifdef COLUMBUS_EDITOR
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}

#define main main
int main(int argc, char** argv)
{
	InitGUI();
	InitWindowAndContext();
	InitRenderGUI(hwnd);

	Scene scene;
	AudioListener Listener;
	Camera camera;
	Renderer MainRender;
	Editor Editor;

	scene.Load("Data/Shadows.scene");

	MainRender.SetMainCamera(camera);
	camera.Pos = Vector3(0, 10, 0);
	camera.Rot = Vector3(0, 180, 0);

	scene.SetCamera(camera);
	scene.SetAudioListener(Listener);

	MSG msg;
	float wheel = 0;
	float RedrawTime = 0.016f;
	bool wasLooking = false;
	while (!quit)
	{
		input.Update();

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) quit = true;
		}
		
		{
			if (Editor.PanelScene.IsHover())
			{
				wheel += input.GetMouseWheel().Y * 5;
			}

			if (input.GetMouseButton(1).State)
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
					/*camera.Pos += camera.Direction() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_W);
					camera.Pos -= camera.Direction() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_S);
					camera.Pos -= camera.Right() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_A);
					camera.Pos += camera.Right() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_D);
					camera.Pos -= camera.Up() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_LSHIFT);
					camera.Pos += camera.Up() * RedrawTime * CameraSpeed * input.GetKey(SDL_SCANCODE_SPACE);

					camera.Rot -= Vector3(0, 0, 120 * RedrawTime) * input.GetKey(SDL_SCANCODE_Q);
					camera.Rot += Vector3(0, 0, 120 * RedrawTime) * input.GetKey(SDL_SCANCODE_E);*/

					Vector2 deltaMouse = input.GetMouseMovement();
					camera.Rot += Vector3(deltaMouse.Y, -deltaMouse.X, 0) * 0.3f;
				}
			}
			else
			{
				wasLooking = false;
			}

			camera.Pos += camera.Direction() * wheel * RedrawTime;
			wheel -= wheel * 3 * RedrawTime;
			if (abs(wheel) <= 0.2) wheel = 0.0f;

			camera.Perspective(60, (float)wnd_size.X / (float)wnd_size.Y, 0.1, 1000);
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

			game.Update();

			MainRender.SetDeltaTime(RedrawTime);
			MainRender.ContextSize = wnd_size;
			MainRender.SetViewport({ 0 }, wnd_size);
			scene.Update();
			MainRender.SetMainCamera(camera);
			MainRender.SetSky(scene.Sky);
			MainRender.SetScene(&scene);
			MainRender.SetRenderList(&scene.Objects.Resources);
			MainRender.Render();

			RenderBeginGUI();
			Editor.Draw(scene, MainRender, wnd_size, 0.016f);
			RenderEndGUI();

			SwapBuffers(hdc);
		}
	}

	ShutdownGUI();
	ShutdownWindowAndContext();

	return 0;
}
