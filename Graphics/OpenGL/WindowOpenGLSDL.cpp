#include <Graphics/OpenGL/WindowOpenGLSDL.h>
#include <RenderAPIOpenGL/OpenGL.h>
#include <System/Log.h>
#include <GL/glew.h>

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
			if (SDL_GetNumVideoDisplays() < 0)
			{
				Log::Fatal("No display");
			}

			SDL_DisplayMode* DisplayModes = new SDL_DisplayMode[SDL_GetNumVideoDisplays()];

			Log::Initialization("SDL2 initialized");
			Log::Initialization("Current video driver: %s", SDL_GetCurrentVideoDriver());
			Log::Initialization("Display count: %i", SDL_GetNumVideoDisplays());

			for (int32 i = 0; i < SDL_GetNumVideoDisplays(); i++)
			{
				if (SDL_GetDesktopDisplayMode(i, &DisplayModes[i]) != 0)
				{
					Log::Fatal("Can't get display info (%i)", i + 1);
				}
				else
				{
					Log::Initialization("Display resolution (%i): %ix%i", i + 1, DisplayModes[i].w, DisplayModes[i].h);
					Log::Initialization("Display refresh rate (%i): %i", i + 1, DisplayModes[i].refresh_rate);
				}
			}

			delete[] DisplayModes;
		}
	}
	
	void WindowOpenGLSDL::InitializeWindow(const iVector2& InSize, const char* Title, Window::Flags F)
	{
		int flags = SDL_WINDOW_OPENGL;
		int pos = SDL_WINDOWPOS_CENTERED;

		if ((uint32)F & (uint32)Window::Flags::Resizable)  flags |= SDL_WINDOW_RESIZABLE;
		if ((uint32)F & (uint32)Window::Flags::Fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

		Window = SDL_CreateWindow(Title, pos, pos, InSize.X, InSize.Y, flags);
		Context = SDL_GL_CreateContext(Window);
	}
	
	void WindowOpenGLSDL::InitializeOpenGL()
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

		int MajorVersion = 3;
		int MinorVersion = 0;

		glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, MajorVersion);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, MinorVersion);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		SDL_GL_SetSwapInterval(1);

		if (glewInit() != GLEW_OK)
		{
			Log::Fatal("Can't initialize GLEW");
		} else
		{
			Log::Initialization("GLEW initialized");
			Log::Initialization("Initialized OpenGL %i.%i", MajorVersion, MinorVersion);
		}

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_TEXTURE_CUBE_MAP_ARB);
		glEnable(GL_ALPHA_TEST);
		//glEnable(GL_MULTISAMPLE);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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
		if (Window && Open)
		{
			SDL_GetWindowSize(Window, &Size.X, &Size.Y);

			while (SDL_PollEvent(&TmpEvent))
			{
				if (TmpEvent.type == SDL_QUIT) Open = false;

				PollEvent(TmpEvent);
			}
		}
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
			glFinish();
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
	
	void WindowOpenGLSDL::SetSize(const iVector2& Size) {}
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
	
	WindowOpenGLSDL::~WindowOpenGLSDL()
	{
		if (Window)
		{
			SDL_GL_DeleteContext(Context);
			SDL_DestroyWindow(Window);
		}
	}

}



