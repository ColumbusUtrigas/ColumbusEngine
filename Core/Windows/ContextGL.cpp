#include "Main.h"
#include <RenderAPIOpenGL/OpenGL.h>
#include <Graphics/Device.h>
#include <Graphics/DebugRender.h>
#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <string>

#include <imgui.h>
#include <imgui/examples/imgui_impl_win32.h>
#include <imgui/examples/imgui_impl_opengl3.h>
#include <ImGuizmo/ImGuizmo.h>

#include <gl/glew.h>
#include <GL/wglew.h>

HDC hdc;
HGLRC hrc;

bool InitializeWindowAndContext_GL()
{
	//we want a 4.3 context
	const int major = 4;
	const int minor = 3;

	WNDCLASS wc = { 0 };
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = window_class;
	if (!RegisterClass(&wc))
		COLUMBUS_ASSERT(false);

	char versionstr[64] = {0};
	snprintf(versionstr, 64, " <GL %i.%i>", major, minor);

	auto title = std::string(window_title) + versionstr;

	hwnd = CreateWindow(window_class, title.c_str(), WS_OVERLAPPEDWINDOW, 6, 12, 800, 600, 0, 0, hinstance, 0);

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
			WGL_CONTEXT_MAJOR_VERSION_ARB, major,
			WGL_CONTEXT_MINOR_VERSION_ARB, minor,
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
	Columbus::OpenGL::Init();
	Columbus::gDevice = new Columbus::DeviceOpenGL();
	Columbus::gDevice->Initialize();

	PFNWGLSWAPINTERVALEXTPROC wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	wglSwapInterval(0);

	ShowWindow(hwnd, SW_SHOWNORMAL);
	return true;
}

#if COLUMBUS_EDITOR
void InitializeImGUI_GL()
{
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplOpenGL3_Init("#version 130");
	Columbus::Graphics::gDebugRender.Initialize();
}

void NewFrameImGUI_GL()
{
	ImGui_ImplOpenGL3_NewFrame();
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void RenderImGUI_GL()
{
	auto& io = ImGui::GetIO();
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ShutdownImGUI_GL()
{

}
#endif

void BeginFrame_GL()
{
	glViewport(0, 0, 800, 600);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Present_GL()
{
	SwapBuffers(hdc);
}

void ShutdownWindowAndContext_GL()
{

}
