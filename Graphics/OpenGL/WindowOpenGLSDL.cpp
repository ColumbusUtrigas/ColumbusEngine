#include <Graphics/OpenGL/WindowOpenGLSDL.h>
#include <System/Log.h>
#include <RenderAPIOpenGL/OpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	static bool IsSDLInitialized = false;

	WindowOpenGLSDL::WindowOpenGLSDL() :
		mWindow(nullptr)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	WindowOpenGLSDL::WindowOpenGLSDL(const Vector2 aSize, const std::string aTitle, const WindowFlags aFlags) :
		mWindow(nullptr)
	{
		create(aSize, aTitle, aFlags);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void WindowOpenGLSDL::initializeSDL()
	{
		if (SDL_Init(SDL_INIT_EVERYTHING))
		{
			Log::fatal("Can't initialize SDL2");
		}
		else
		{
			if (SDL_GetNumVideoDisplays() < 0)
			{
				Log::fatal("No display");
			}

			SDL_DisplayMode* DisplayModes = new SDL_DisplayMode[SDL_GetNumVideoDisplays()];

			Log::initialization("SDL2 initialized");
			Log::initialization("Current video driver: " + std::string(SDL_GetCurrentVideoDriver()));
			Log::initialization("Display count: " + std::to_string(SDL_GetNumVideoDisplays()));

			for (int32 i = 0; i < SDL_GetNumVideoDisplays(); i++)
			{
				if (SDL_GetDesktopDisplayMode(i, &DisplayModes[i]) != 0)
				{
					Log::fatal("Can't get display info (" + std::to_string(i + 1) + ")");
				}
				else
				{
					Log::initialization("Display resolution (" + std::to_string(i + 1) + std::string(")") + std::string(": ") + std::to_string(DisplayModes[i].w) + std::string("x") + std::to_string(DisplayModes[i].h));
					Log::initialization("Display refresh rate (" + std::to_string(i + 1) + std::string(")") + std::string(": ") + std::to_string(DisplayModes[i].refresh_rate));
				}
			}

			delete[] DisplayModes;
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void WindowOpenGLSDL::initializeWindow(const Vector2 aSize, const std::string aTitle, const WindowFlags aFlags)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
		SDL_GL_SetSwapInterval(1);

		int flags = SDL_WINDOW_OPENGL;
		int pos = SDL_WINDOWPOS_CENTERED;

		switch (aFlags)
		{
		case E_WINDOW_FLAG_RESIZABLE: flags |= SDL_WINDOW_RESIZABLE; break;
		case E_WINDOW_FLAG_FULLSCREEN: flags |= SDL_WINDOW_FULLSCREEN; break;
		case E_WINDOW_FLAG_FULLSCREEN_DESKTOP: flags |= SDL_WINDOW_FULLSCREEN_DESKTOP; break;
		}

		mWindow = SDL_CreateWindow(aTitle.c_str(), pos, pos, aSize.X, aSize.Y, flags);
		mGLC = SDL_GL_CreateContext(mWindow);
	}
	//////////////////////////////////////////////////////////////////////////////
	void WindowOpenGLSDL::initializeOpenGL()
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

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_TEXTURE_CUBE_MAP_ARB);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		//glEnable(GL_DITHER);

		if (glewInit() != GLEW_OK)
		{
			Log::fatal("Can't initialize GLEW");
		} else
		{
			Log::initialization("GLEW initialized");
			Log::initialization("Initialized OpenGL " + std::to_string(MajorVersion) + "." + std::to_string(MinorVersion));
		}

		OpenGL::Init();
	}
	//////////////////////////////////////////////////////////////////////////////
	void WindowOpenGLSDL::getVersions()
	{
		SDL_version cVer;
		SDL_version lVer;

		SDL_VERSION(&cVer);
		SDL_GetVersion(&lVer);

		Log::initialization("SDL version: %d.%d.%d", cVer.major, cVer.minor, cVer.patch);
		Log::initialization("SDL linked version: %d.%d.%d", lVer.major, lVer.minor, lVer.patch);
		Log::initialization("OpenGL version: " + stdstr((const char*)glGetString(GL_VERSION)));
		Log::initialization("OpenGL vendor: " + stdstr((const char*)glGetString(GL_VENDOR)));
		Log::initialization("OpenGL renderer: " + stdstr((const char*)glGetString(GL_RENDERER)));
		Log::initialization("GLSL version: " + stdstr((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)) + "\n");
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool WindowOpenGLSDL::create(const Vector2 aSize, const std::string aTitle, const WindowFlags aFlags)
	{
		if (!IsSDLInitialized)
		{
			initializeSDL();
		}

		initializeWindow(aSize, aTitle, aFlags);

		glewExperimental = GL_TRUE;

		if (!IsSDLInitialized)
		{
			initializeOpenGL();
			getVersions();
			IsSDLInitialized = true;
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void WindowOpenGLSDL::update()
	{
		int x, y;
		SDL_GetWindowSize(mWindow, &x, &y);
		mSize.X = x;
		mSize.Y = y;

		while(SDL_PollEvent(&mTmpEvent))
		{
			if(mTmpEvent.type == SDL_QUIT) mOpen = false;

			pollEvent(mTmpEvent);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void WindowOpenGLSDL::clear(const Vector4 aColor)
	{
		SDL_GL_MakeCurrent(mWindow, mGLC);
		SDL_GL_SetSwapInterval(mVSync ? 1 : 0);
		glClearColor(aColor.X, aColor.Y, aColor.Z, aColor.W);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, Math::TruncToInt(getSize().X), Math::TruncToInt(getSize().Y));
	}
	//////////////////////////////////////////////////////////////////////////////
	void WindowOpenGLSDL::display()
	{
		SDL_GL_SwapWindow(mWindow);
		glFinish();
		mRedrawTime = Math::TruncToFloat(mRedrawTimer.elapsed());
		mRedrawTimer.reset();
		mFrames++;
		if (mFPSTimer.elapsed() >= 1.0f)
		{
			mFPS = mFrames;
			mFrames = 0;
			mFPSTimer.reset();
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void WindowOpenGLSDL::setVSync(const bool aVSync)
	{
		mVSync = static_cast<bool>(aVSync);
	}
	//////////////////////////////////////////////////////////////////////////////
	void WindowOpenGLSDL::setSize(const Vector2 aSize)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	SDL_Window* WindowOpenGLSDL::getHandle() const
	{
		return mWindow;
	}
	//////////////////////////////////////////////////////////////////////////////
	void WindowOpenGLSDL::pollEvent(SDL_Event& aEvent)
	{
		if (!mWindow) return;

		if(aEvent.type == SDL_WINDOWEVENT && aEvent.window.windowID == SDL_GetWindowID(mWindow))
		{
			switch (aEvent.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				SDL_HideWindow(mWindow);
				SDL_LockAudio();
				mOpen = false;
				break;
			case SDL_WINDOWEVENT_SHOWN:
				mShown = true;
				break;
			case SDL_WINDOWEVENT_HIDDEN:
				mShown = false;
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				mMinimized = true;
				mMaximized = false;
				break;
			case SDL_WINDOWEVENT_MAXIMIZED:
				mMinimized = false;
				mMaximized = true;
				break;
			case SDL_WINDOWEVENT_ENTER:
				mMouseFocus = true;
				break;
			case SDL_WINDOWEVENT_LEAVE:
				mMouseFocus = false;
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				mKeyFocus = true;
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				mKeyFocus = false;
				break;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	std::string WindowOpenGLSDL::getType() const
	{
		return "WindowOpenGLSDL";
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	WindowOpenGLSDL::~WindowOpenGLSDL()
	{

	}

}








