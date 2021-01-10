#include "Main.h"

#include <Core/Windows/WindowsDragDrop.h>

#include <Scene/Scene.h>
#include <Editor/Editor.h>
#include <Input/Input.h>
#include <imgui/imgui.h>
#include <Profiling/Profiling.h>
#include <Graphics/Device.h>
#include <Core/Game.h>

#include <System/File.h>
#include <Common/JSON/JSON.h>
#include <Graphics/ShaderCompiler.h>
#include <fstream>
#include <iostream>

//Hint to the driver to use discrete GPU
extern "C"
{
	//NVIDIA
	__declspec(dllexport) int NvOptimusEnablement = 1;
	//AMD
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

LPCTSTR window_class = "ColumbusEngineWinClass";
LPCTSTR window_title = "Columbus Engine";
HWND hwnd;
HINSTANCE hinstance;

bool Running = true;

Columbus::GraphicsAPI chosen_api = Columbus::GraphicsAPI::None;
Columbus::Buffer* vbuf;
Columbus::Buffer* cbuf;
Columbus::Buffer* ibuf;
Columbus::Graphics::GraphicsPipeline* pipeline;

// legacy
Columbus::InputLayout* layout;
Columbus::ShaderProgram* shader;

static std::string ReadFile(std::string_view path)
{
	std::ifstream f(path.data());
	return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

void TriangleInit()
{
	using namespace Columbus;

	Vector2 vList[] = {
			{ 0.0f, 0.5f },
			{ 0.5f, -0.5f },
			{ -0.5f, -0.5f },
	};

	Vector4 cList[] = {
		{ 1, 0, 0, 1 },
		{ 0, 1, 0, 1 },
		{ 0, 0, 1, 1}
	};

	// vertex
	{
		BufferDesc desc;
		desc.Size = sizeof(vList);
		desc.BindFlags = BufferType::Array;
		desc.Usage = BufferUsage::Static;
		desc.CpuAccess = BufferCpuAccess::Write;

		SubresourceData initial;
		initial.pSysMem = vList;
		initial.SysMemPitch = sizeof(vList);

		auto device = gDevice;
		device->CreateBuffer(desc, &initial, &vbuf);
	}

	// color
	{
		BufferDesc desc;
		desc.Size = sizeof(cList);
		desc.BindFlags = BufferType::Array;
		desc.Usage = BufferUsage::Static;
		desc.CpuAccess = BufferCpuAccess::Write;

		SubresourceData initial;
		initial.pSysMem = cList;
		initial.SysMemPitch = sizeof(cList);

		gDevice->CreateBuffer(desc, &initial, &cbuf);
	}

	// index
	{
		int indices[] = { 0, 1, 2 };

		BufferDesc desc;
		desc.Size = sizeof(indices);
		desc.BindFlags = BufferType::Index;
		desc.Usage = BufferUsage::Static;
		desc.CpuAccess = BufferCpuAccess::Write;

		SubresourceData initial;
		initial.pSysMem = indices;
		initial.SysMemPitch = sizeof(indices);

		gDevice->CreateBuffer(desc, &initial, &ibuf);
	}

	// graphics pipeline
	{
		Graphics::GraphicsPipelineDesc desc;
		desc.layout.Elements = {
			InputLayoutElementDesc{ "POSITION", 0, 0, 2 },
			InputLayoutElementDesc{ "COLOR", 0, 1, 4 },
			InputLayoutElementDesc{ "TEXCOORD", 0, 2, 2 },
		};
		desc.rasterizerState.Cull = CullMode::No;
		desc.topology = PrimitiveTopology::TriangleList;

		ShaderLanguage lang;

		switch (chosen_api)
		{
		case GraphicsAPI::DX12:
			lang = ShaderLanguage::HLSL;
			//desc.VS = std::make_shared<ShaderStage>(ReadFile("vertex.hlsl"), "main", ShaderType::Vertex, ShaderLanguage::HLSL);
			//desc.HS = std::make_shared<ShaderStage>(ReadFile("hull.hlsl"), "main", ShaderType::Hull, ShaderLanguage::HLSL);
			//desc.DS = std::make_shared<ShaderStage>(ReadFile("domain.hlsl"), "main", ShaderType::Domain, ShaderLanguage::HLSL);
			//desc.GS = std::make_shared<ShaderStage>(ReadFile("geometry.hlsl"), "main", ShaderType::Geometry, ShaderLanguage::HLSL);
			//desc.PS = std::make_shared<ShaderStage>(ReadFile("pixel.hlsl"), "main", ShaderType::Pixel, ShaderLanguage::HLSL);
			break;

		case GraphicsAPI::OpenGL:
			lang = ShaderLanguage::GLSL;
			//desc.VS = std::make_shared<ShaderStage>(ReadFile("vertex.glsl"), "main", ShaderType::Vertex, ShaderLanguage::GLSL);
			//desc.HS = std::make_shared<ShaderStage>(ReadFile("hull.glsl"), "main", ShaderType::Hull, ShaderLanguage::GLSL);
			//desc.DS = std::make_shared<ShaderStage>(ReadFile("domain.glsl"), "main", ShaderType::Domain, ShaderLanguage::GLSL);
			//desc.GS = std::make_shared<ShaderStage>(ReadFile("geometry.glsl"), "main", ShaderType::Geometry, ShaderLanguage::GLSL);
			//desc.PS = std::make_shared<ShaderStage>(ReadFile("pixel.glsl"), "main", ShaderType::Pixel, ShaderLanguage::GLSL);
			break;
		}

		auto prog = Graphics::ShaderCompiler::Compile(ReadFile("shader.csl"), lang, {});
		desc.VS = prog.VS;
		desc.PS = prog.PS;

		gDevice->CreateGraphicsPipeline(desc, &pipeline);
	}
}

int main()
{
	Game game;

	chosen_api = Columbus::GraphicsAPI::OpenGL;

	Columbus::Editor::Settings settings;
	settings.TryLoad();

	if (!InitializeEngine())
	{
		COLUMBUS_ASSERT(false);
		return 1;
	}

	game.EngineStarted();

	#if COLUMBUS_EDITOR
		Columbus::Editor::Editor editor;
		Columbus::Scene scene;
		Columbus::Renderer render;
		Columbus::AudioListener listener;

		editor.settings = settings;

		//scene.SetCamera(editor.camera);
		//scene.SetAudioListener(listener);
		//scene.Audio.Play();

		//columbus_editor_main();
	#endif

	float RedrawTime = 0;
	Columbus::Timer RedrawTimer;
	Columbus::iVector2 viewport_size{ 800, 600 };

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	TriangleInit();

 	while (Running)
	{
		Columbus::ResetProfiling();
		PROFILE_CPU(Columbus::ProfileModule::CPU);

		RedrawTime = RedrawTimer.Elapsed();
		RedrawTimer.Reset();

		Columbus::gInput.Update();
		Columbus::gInput.SetKeyboardFocus(true);

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		/*editor.camera.Perspective(60, (float)viewport_size.X / (float)viewport_size.Y, 0.1, 1000);
		editor.camera.Update();

		scene.Update();*/

		game.Update();

		BeginFrame();

		/*render.SetDeltaTime(RedrawTime);
		render.ContextSize = viewport_size;
		render.SetViewport({ 0 }, viewport_size);
		scene.Update();
		render.SetMainCamera(editor.camera);
		render.SetSky(scene.Sky);
		render.SetScene(&scene);
		render.SetRenderList(&scene.Objects.Resources);*/
		//render.Render();

		NewFrameImGUI();
		Columbus::Graphics::gDebugRender.NewFrame();

		editor.Draw(scene, render, viewport_size, RedrawTime);

		/*Columbus::Buffer* bufs[] = { vbuf, cbuf };
		Columbus::Viewport viewport{ 0, 0, (float)viewport_size.X, (float)viewport_size.Y, 0, 1 };
		Columbus::ScissorRect scissor{ 0, 0, (Columbus::uint32)viewport_size.X, (Columbus::uint32)viewport_size.Y };

		Columbus::gDevice->BeginMarker("Draw");
		Columbus::gDevice->SetGraphicsPipeline(pipeline);
		Columbus::gDevice->RSSetViewports(1, &viewport);
		Columbus::gDevice->RSSetScissorRects(1, &scissor);
		Columbus::gDevice->IASetVertexBuffers(0, 1, bufs);
		Columbus::gDevice->IASetIndexBuffer(ibuf, Columbus::IndexFormat::Uint32, 0);
		Columbus::gDevice->Draw(3, 0);
		Columbus::gDevice->EndMarker();*/

		Render();
	}

	game.Shutdown();
	//editor.settings.TrySave();
	ShutdownEngine();

	return 0;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

IDragDropManager* _dropManager = new WindowsDragDropManager();

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	using namespace Columbus;

	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wparam, lparam);

	LRESULT result = 0;
	Event e;
	memset(&e, 0, sizeof(e));

	BOOL attrubute = TRUE;
	switch (msg)
	{
	case WM_CREATE:
		_dropManager->onDrop = []() { printf("drop\n"); };

		OleInitialize(NULL);
		RegisterDragDrop(hWnd, (WindowsDragDropManager*)_dropManager);
		break;
	case WM_NCCREATE:
		return DefWindowProc(hWnd, msg, wparam, lparam);
	case WM_DESTROY:
		Running = false;
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		ResizeSwapchain_DX12(hwnd, wparam, lparam);
		//wnd_size = { LOWORD(lparam), HIWORD(lparam) };
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
		e.MouseWheel.Y = (float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA;
		printf("%f\n", e.MouseWheel.Y);
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

	gInput.PollEvent(e);

	return result;
}

#if 0

#include <Scene/Scene.h>
#include <Input/Input.h>
#include <Editor/Editor.h>
#include <Profiling/Profiling.h>
#include <Core/Game.h>
#include <Core/FileDialog.h>
#include <Common/JSON/JSON.h>

#include <Graphics/OpenGL/DeviceOpenGL.h>
//#include <Graphics/Vulkan/InstanceVulkan.h>
#include <GLEW/include/GL/wglew.h>

using namespace Columbus;

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <imgui/examples/imgui_impl_opengl3.h>
#include <imgui/examples/imgui_impl_win32.h>

#include <Windows.h>
#include <uxtheme.h>
#include <dwmapi.h>

#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Dwmapi.lib")

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

#define DWMWA_USE_IMMERSIVE_DARK_MODE 19
#define COLUMBUS_EDITOR

HWND hwnd;
HDC hdc;
HGLRC hrc;

Game game;
Input input;

bool quit = false;
const char* class_name = "Columbus Engine Window Class";
iVector2 wnd_size = { 640, 480 };
bool wnd_max = false;

IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wparam, lparam);

	LRESULT result = 0;
	Event e;
	memset(&e, 0, sizeof(e));

	BOOL attrubute = TRUE;
	switch (msg)
	{
	case WM_NCCREATE:
		SetWindowTheme(hWnd, L"DarkMode_Explorer", NULL);
		DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &attrubute, sizeof(BOOL));
		return DefWindowProc(hWnd, msg, wparam, lparam);
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

/*void InitEditorConfigs(Editor::Settings& settings)
{
#ifdef COLUMBUS_EDITOR
	JSON j;
	if (j.Load("editor_settings.json"))
	{
		settings.Deserialize(j);
	}
	else
	{
		settings.Serialize(j);
		j.Save("editor_settings.json");
	}

	wnd_size = settings.windowSize;
	wnd_max = settings.windowMaximized;
#endif
}*/

/*void SaveEditorConfigs(const Editor::Settings& settings)
{
#ifdef COLUMBUS_EDITOR
	JSON j;
	settings.Serialize(j);
	j.Save("editor_settings.json");
#endif
}*/

/*void InitGUI()
{
#ifdef COLUMBUS_EDITOR
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
#endif
}*/

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
	Graphics::gDebugRender.Initialize();
#endif
}

/*void ShowWindow()
{
	//SetWindowTheme(hwnd, L"DarkMode_Explorer", NULL);
	ShowWindow(hwnd, wnd_max ? SW_MAXIMIZE : SW_SHOWNORMAL);
}*/

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
	Graphics::gDebugRender.Shutdown();
#endif
}

#undef main
int main(int argc, char** argv)
{
	Editor::Settings settings;

	InitEditorConfigs(settings);
	InitGUI();
	InitWindowAndContext();
	InitRenderGUI(hwnd);
	ShowWindow();

	Scene scene;
	AudioListener Listener;
	Camera camera;
	Renderer MainRender;
	Editor::Editor Editor;
	Editor.settings = settings;

	//scene.Load("Data/Shadows.scene");

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

			//Graphics::gDebugRender.RenderBox(Transform({}, { 45 }, { 1 }).GetMatrix(), { 0.8, 0.5, 0.5, 1 }, true);
			//Graphics::gDebugRender.RenderSphere({5,0,0}, 1, { 0.8, 0.5, 0.5, 1 }, true);

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

			Graphics::gDebugRender.NewFrame();
			RenderBeginGUI();
			Editor.Draw(scene, MainRender, wnd_size, RedrawTime);
			RenderEndGUI();

			SwapBuffers(hdc);
		}
	}

	ShutdownGUI();
	ShutdownWindowAndContext();
	SaveEditorConfigs(Editor.settings);

	return 0;
}

#endif