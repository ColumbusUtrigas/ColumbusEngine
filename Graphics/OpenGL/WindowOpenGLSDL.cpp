#include <Graphics/OpenGL/WindowOpenGLSDL.h>
#include <RenderAPIOpenGL/OpenGL.h>
#include <System/Log.h>
#include <GL/glew.h>

#include <Lib/corinfo/corinfo.h> // my own library to get system info

#define CONTEXT_MAJOR_VERSION 3
#define CONTEXT_MINOR_VERSION 3

namespace Columbus
{

	static bool IsSDLInitialized = false;

	WindowOpenGLSDL::WindowOpenGLSDL() {}
	WindowOpenGLSDL::WindowOpenGLSDL(const iVector2& InSize, const char* Title, Window::Flags F)
	{
		Create(InSize, Title, F);
	}
	
	void WindowOpenGLSDL::InitializeSDL()
	{
		if (SDL_Init(SDL_INIT_EVERYTHING))
		{
			Log::Fatal("Can't initialize SDL2");
		}
		else
		{
			struct corinfo info;
			if (corinfo_GetInfo(&info) == -1)
			{
				Log::Error("Could not get system info");
			}

			#define CPU_FEATURE_YESNO(feature) info.Cpu.feature ? "yes" : "no"

			Log::Initialization("CPU info: %i cores, %i MHz\n%s\n"
				"MMX: %s\n"
				"SSE: %s\n"
				"SSE2: %s\n"
				"SSE3: %s\n"
				"SSE41: %s\n"
				"SSE42: %s\n"
				"AVX: %s\n",
				info.Cpu.Count, info.Cpu.Frequency,
				info.Cpu.Name,
				CPU_FEATURE_YESNO(MMX),
				CPU_FEATURE_YESNO(SSE),
				CPU_FEATURE_YESNO(SSE2),
				CPU_FEATURE_YESNO(SSE3),
				CPU_FEATURE_YESNO(SSE41),
				CPU_FEATURE_YESNO(SSE42),
				CPU_FEATURE_YESNO(AVX)
			);

			Log::Initialization("RAM info:\nTotal: %i KB\nFree: %i KB\nUsage: %i%%\n",
				info.Ram.Total, info.Ram.Free, info.Ram.Usage
			);
			Log::Initialization("HDD info:\nTotal: %i KB\nFree: %i KB\nUsage: %i%%\n",
				info.Hdd.Total, info.Hdd.Free, info.Hdd.Usage
			);

			if (SDL_GetNumVideoDisplays() < 0)
			{
				Log::Fatal("No display");
			}

			Log::Initialization("SDL2 initialized");
			Log::Initialization("Current video driver: %s", SDL_GetCurrentVideoDriver());
			Log::Initialization("Display count: %i", SDL_GetNumVideoDisplays());

			for (int32 i = 0; i < SDL_GetNumVideoDisplays(); i++)
			{
				SDL_DisplayMode DisplayMode;
				if (SDL_GetDesktopDisplayMode(i, &DisplayMode) != 0)
				{
					Log::Fatal("Can't get display info (%i)", i + 1);
				}
				else
				{
					Log::Initialization("Display resolution (%i): %ix%i", i + 1, DisplayMode.w, DisplayMode.h);
					Log::Initialization("Display refresh rate (%i): %i", i + 1, DisplayMode.refresh_rate);
				}
			}
		}
	}
	
	void WindowOpenGLSDL::InitializeWindow(const iVector2& InSize, const char* Title, Window::Flags F)
	{
		int flags = SDL_WINDOW_OPENGL;
		int pos = SDL_WINDOWPOS_CENTERED;

		if ((uint32)F & (uint32)Window::Flags::Resizable)  flags |= SDL_WINDOW_RESIZABLE;
		if ((uint32)F & (uint32)Window::Flags::Fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, CONTEXT_MAJOR_VERSION);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, CONTEXT_MINOR_VERSION);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		SDL_GL_SetSwapInterval(1);

		Window = SDL_CreateWindow(Title, pos, pos, InSize.X, InSize.Y, flags);
		Context = SDL_GL_CreateContext(Window);
	}
	
	void WindowOpenGLSDL::InitializeOpenGL()
	{
		int MajorVersion = CONTEXT_MAJOR_VERSION;
		int MinorVersion = CONTEXT_MINOR_VERSION;

		glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);

		SDL_GL_SetSwapInterval(1);

		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			Log::Fatal("Can't initialize GLEW");
		} else
		{
			Log::Initialization("GLEW initialized");
			Log::Initialization("Initialized OpenGL %i.%i", MajorVersion, MinorVersion);
		}

		OpenGL::Init();
	}
	
	void WindowOpenGLSDL::GetVersions()
	{
		SDL_version cVer;
		SDL_version lVer;

		SDL_VERSION(&cVer);
		SDL_GetVersion(&lVer);

		Log::Initialization("SDL version: %d.%d.%d", cVer.major, cVer.minor, cVer.patch);
		Log::Initialization("SDL linked version: %d.%d.%d", lVer.major, lVer.minor, lVer.patch);
		Log::Initialization("OpenGL version: %s", glGetString(GL_VERSION));
		Log::Initialization("OpenGL vendor: %s", glGetString(GL_VENDOR));
		Log::Initialization("OpenGL renderer: %s",glGetString(GL_RENDERER));
		Log::Initialization("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	}
	
	bool WindowOpenGLSDL::Create(const iVector2& InSize, const char* Title, Window::Flags F)
	{
		if (!IsSDLInitialized)
		{
			InitializeSDL();
		}

		InitializeWindow(InSize, Title, F);

		glewExperimental = GL_TRUE;

		if (!IsSDLInitialized)
		{
			InitializeOpenGL();
			GetVersions();
			IsSDLInitialized = true;
		}

		return true;
	}

	void WindowOpenGLSDL::Close()
	{
		if (Window && Open)
		{
			SDL_GL_DeleteContext(Context);
			SDL_DestroyWindow(Window);
			Open = false;
		}
	}
	
	void WindowOpenGLSDL::Update()
	{
		
	}
	
	void WindowOpenGLSDL::Clear(const Vector4& Color)
	{
		if (Window && Open)
		{
			SDL_GL_MakeCurrent(Window, Context);
			SDL_GL_SetSwapInterval(VSync ? 1 : 0);
			glClearColor(Color.X, Color.Y, Color.Z, Color.W);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, GetSize().X, GetSize().Y);
		}
	}
	
	void WindowOpenGLSDL::Display()
	{
		if (Window && Open)
		{
			SDL_GL_SwapWindow(Window);
			//glFinish();
			RedrawTime = (float)RedrawTimer.Elapsed();
			RedrawTimer.Reset();
			Frames++;

			if (FPSTimer.Elapsed() >= 1.0f)
			{
				FPS = Frames;
				Frames = 0;
				FPSTimer.Reset();
			}
		}
	}
	
	void WindowOpenGLSDL::SetSize(const iVector2& Size)
	{
		if (Window && Open)
		{
			SDL_SetWindowSize(Window, Size.X, Size.Y);
		}
	}

	void WindowOpenGLSDL::SetMousePosition(const iVector2& Pos)
	{
		if (Window && Open)
		{
			SDL_WarpMouseInWindow(Window, Pos.X, Pos.Y);
		}
	}
	
	void WindowOpenGLSDL::PollEvent(SDL_Event& Event)
	{
		if (Window && Open)
		{
			if (Event.type == SDL_WINDOWEVENT && Event.window.windowID == SDL_GetWindowID(Window))
			{
				switch (Event.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					SDL_HideWindow(Window);
					SDL_LockAudio();
					Open = false;
					break;
				case SDL_WINDOWEVENT_SHOWN:  Shown = true;  break;
				case SDL_WINDOWEVENT_HIDDEN: Shown = false; break;
				case SDL_WINDOWEVENT_MINIMIZED:
					Minimized = true;
					Maximized = false;
					break;
				case SDL_WINDOWEVENT_MAXIMIZED:
					Minimized = false;
					Maximized = true;
					break;
				case SDL_WINDOWEVENT_ENTER:        MouseFocus = true;  break;
				case SDL_WINDOWEVENT_LEAVE:        MouseFocus = false; break;
				case SDL_WINDOWEVENT_FOCUS_GAINED: KeyFocus = true;  break;
				case SDL_WINDOWEVENT_FOCUS_LOST:   KeyFocus = false; break;
				}
			}
		}
	}

	void WindowOpenGLSDL::PollEvent(const Event& E)
	{
		if (Window && Open)
		{
			if (Window == E.Window.Window)
			{
				switch (E.Window.Type)
				{
				case WindowEvent::Type_None: break;
				case WindowEvent::Type_Close:
					SDL_HideWindow(Window);
					SDL_LockAudio();
					Open = false;
					break;
				case WindowEvent::Type_Shown:  Shown = true;  break;
				case WindowEvent::Type_Hidden: Shown = false; break;
				case WindowEvent::Type_Minimized:
					Minimized = true;
					Maximized = false;
					break;
				case WindowEvent::Type_Maximized:
					Minimized = false;
					Maximized = true;
					break;
				case WindowEvent::Type_MouseEnter:        MouseFocus = true;  break;
				case WindowEvent::Type_MouseLeave:        MouseFocus = false; break;
				case WindowEvent::Type_KeyboardFocusGained: KeyFocus = true;  break;
				case WindowEvent::Type_KeyboardFocusLost:   KeyFocus = false; break;
				case WindowEvent::Type_Resize: Size = { E.Window.Data1, E.Window.Data2 }; break;
				}
			}
		}
	}

	SDL_Window* WindowOpenGLSDL::GetWindowHandle() const
	{
		return Window;
	}

	SDL_GLContext WindowOpenGLSDL::GetContextHandle() const
	{
		return Context;
	}
	
	WindowOpenGLSDL::~WindowOpenGLSDL()
	{
		if (Window)
		{
			SDL_GL_DeleteContext(Context);
			SDL_DestroyWindow(Window);
		}
	}

}



