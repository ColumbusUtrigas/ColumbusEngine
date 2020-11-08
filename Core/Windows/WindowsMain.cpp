#if 1

#include <Scene/Scene.h>
#include <Input/Input.h>
#include <Editor/Editor.h>
#include <Profiling/Profiling.h>
#include <Core/Game.h>

#include <Graphics/OpenGL/DeviceOpenGL.h>
//#include <Graphics/Vulkan/InstanceVulkan.h>
#include <GLEW/include/GL/wglew.h>

using namespace Columbus;

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <imgui/examples/imgui_impl_opengl3.h>
#include <imgui/examples/imgui_impl_win32.h>
#include <Windows.h>

#include <Core/FileDialog.h>

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

	LRESULT result = 0;
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
		e.MouseWheel.Y =(float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA;
		e.Type = Event::Type_MouseWheel;
		break;
	case WM_MOUSEHWHEEL:
		e.MouseWheel.X = (float)HIWORD(lparam);
		e.Type = Event::Type_MouseWheel;
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		e.Type = Event::Type_Key;
		e.Key.Code = wparam;
		e.Key.Pressed = true;
		e.Key.Repeat = lparam & 0x40000000 ? true : false;
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		e.Type = Event::Type_Key;
		e.Key.Code = wparam;
		e.Key.Pressed = false;
		e.Key.Repeat = false;
		break;
	case WM_SETCURSOR:
		if (ImGui::GetMouseCursor() == ImGuiMouseCursor_None || ImGui::GetMouseCursor() == ImGuiMouseCursor_Arrow)
			DefWindowProc(hWnd, msg, wparam, lparam);
		break;
	default:
		result = DefWindowProc(hWnd, msg, wparam, lparam);
		break;
	}

	input.PollEvent(e);

	return result;
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

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR), // Size Of This Pixel Format Descriptor
		1,
		PFD_DRAW_TO_WINDOW | // Format Must Support Window
			PFD_SUPPORT_OPENGL | // Format Must Support OpenGL
			PFD_DOUBLEBUFFER,
		(BYTE)PFD_TYPE_RGBA,
		(BYTE)24,
		(BYTE)0, (BYTE)0, (BYTE)0, (BYTE)0, (BYTE)0, (BYTE)0, // Color Bits Ignored
		(BYTE)0, // Alpha Buffer
		(BYTE)0, // Shift Bit Ignored
		(BYTE)0, // No Accumulation Buffer
		(BYTE)0, (BYTE)0, (BYTE)0, (BYTE)0, // Accumulation Bits Ignored
		(BYTE)24, // 24Bit Z-Buffer (Depth Buffer)
		(BYTE)0, // No Stencil Buffer
		(BYTE)0, // No Auxiliary Buffer
		(BYTE)PFD_MAIN_PLANE, // Main Drawing Layer
		(BYTE)0, // Reserved
		0, 0, 0 // Layer Masks Ignored
	};

	hdc = GetDC(hwnd);

	int fmt = ChoosePixelFormat(hdc, &pfd);
	COLUMBUS_ASSERT(fmt != 0);
	if (SetPixelFormat(hdc, fmt, &pfd) != TRUE) COLUMBUS_ASSERT(false);

	hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);

	if (true)
	{
		// OpenGL3+ init

		int attribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4, //we want a 4.3 context
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			//and it shall be forward compatible so that we can only use up to date functionality
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB /*| _WGL_CONTEXT_DEBUG_BIT_ARB*/,
			0
		}; //zero indicates the end of the array

		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr; //pointer to the method
		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
		HGLRC new_hrc = wglCreateContextAttribsARB(hdc, 0, attribs);
		wglMakeCurrent(hdc, nullptr);
		wglDeleteContext(hrc);
		hrc = new_hrc;

		wglMakeCurrent(hdc, hrc);
	}

	printf("%s\n", glGetString(GL_VERSION));

	ShowWindow(hwnd, SW_SHOWNORMAL);

	glewInit();
	OpenGL::Init();
	gDevice = new DeviceOpenGL();
	gDevice->Initialize();

	PFNWGLSWAPINTERVALEXTPROC wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	wglSwapInterval(0);
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
	glClear(GL_COLOR_BUFFER_BIT);
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

#undef main
int main(int argc, char** argv)
{
	InitGUI();
	InitWindowAndContext();
	InitRenderGUI(hwnd);

	Scene scene;
	AudioListener Listener;
	Camera camera;
	Renderer MainRender;
	Editor::Editor Editor;

	scene.Load("Data/Shadows.scene");

	MainRender.SetMainCamera(camera);
	camera.Pos = Vector3(0, 10, 0);
	camera.Rot = Vector3(0, 180, 0);

	scene.SetCamera(camera);
	scene.SetAudioListener(Listener);
	scene.Audio.Play();

	MSG msg;
	float wheel = 0;
	float RedrawTime = 0;
	bool wasLooking = false;
	const int CameraSpeed = 5;

	Timer RedrawTimer;

	while (!quit)
	{
		ResetProfiling();
		PROFILE_CPU(ProfileModule::CPU);
		RedrawTime = RedrawTimer.Elapsed();
		RedrawTimer.Reset();

		input.Update();
		input.SetKeyboardFocus(true);

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) quit = true;
		}
		
		{
			if (Editor.panelScene.IsHover())
			{
				wheel = input.GetMouseWheel().Y * 5;
			}

			if (input.GetMouseButton(1).State)
			{
#ifdef COLUMBUS_EDITOR
				if (Editor.panelScene.IsHover())
				{
					wasLooking = true;
				}
#else`
				wasLooking = true;
#endif

				if (wasLooking)
				{
					camera.Pos += camera.Direction() * RedrawTime * CameraSpeed * input.GetKey('W');
					camera.Pos -= camera.Direction() * RedrawTime * CameraSpeed * input.GetKey('S');
					camera.Pos -= camera.Right() * RedrawTime * CameraSpeed * input.GetKey('A');
					camera.Pos += camera.Right() * RedrawTime * CameraSpeed * input.GetKey('D');
					camera.Pos -= camera.Up() * RedrawTime * CameraSpeed * input.GetKey(VK_SHIFT);
					camera.Pos += camera.Up() * RedrawTime * CameraSpeed * input.GetKey(VK_SPACE);

					camera.Rot -= Vector3(0, 0, 120 * RedrawTime) * input.GetKey('Q');
					camera.Rot += Vector3(0, 0, 120 * RedrawTime) * input.GetKey('E');

					Vector2 deltaMouse = input.GetMouseMovement();
					camera.Rot += Vector3(deltaMouse.Y, -deltaMouse.X, 0) * 0.3f;
				}
			}
			else
			{
				wasLooking = false;
			}

			if (Editor.panelScene.IsHover() && input.GetMouseButton(2).State)
			{
				auto deltaMouse = input.GetMouseMovement();
				camera.Pos -= camera.Right() * deltaMouse.X * 0.1f;
				camera.Pos += camera.Up() * deltaMouse.Y * 0.1f;
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
			}

			camera.Pos += camera.Direction() * wheel;
			//wheel -= wheel * 3 * RedrawTime;
			//if (abs(wheel) <= 0.2) wheel = 0.0f;

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
			Editor.Draw(scene, MainRender, wnd_size, RedrawTime);
			RenderEndGUI();

			SwapBuffers(hdc);
		}
	}

	ShutdownGUI();
	ShutdownWindowAndContext();

	return 0;
}

#endif