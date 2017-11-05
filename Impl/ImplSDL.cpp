/************************************************
*                  ImplSDL.cpp                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   31.10.2017                  *
*************************************************/

#include <Impl/ImplSDL.h>

namespace Columbus
{

	static bool C_SDL_INITED = false;

	#define C_BUTTON_LEFT SDL_BUTTON_LEFT
	#define C_BUTTON_MIDDLE SDL_BUTTON_MIDDLE
	#define C_BUTTON_RIGHT SDL_BUTTON_RIGHT
	#define C_BUTTON_X1 SDL_BUTTON_X1
	#define C_BUTTON_X2 SDL_BUTTON_X2
	//////////////////////////////////////////////////////////////////////////////
	//Show simple message box, modal of window
	void C_ShowMessageBox(C_MessageBoxType aType, std::string aTitle,
		std::string aMessage, C_SDLWindow* aWindow)
	{
		SDL_Window* win = nullptr;
		unsigned int type;
		if (aWindow != nullptr)
			win = aWindow->getHandle();
		switch (aType)
		{
		case C_MB_INFO:    type = SDL_MESSAGEBOX_INFORMATION; break;
		case C_MB_WARNING: type = SDL_MESSAGEBOX_WARNING;     break;
		case C_MB_ERROR:   type = SDL_MESSAGEBOX_ERROR;       break;
		}

		SDL_ShowSimpleMessageBox(type, aTitle.c_str(), aMessage.c_str(), win);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Show complex message box, modal of window
	void C_ShowMessageBox(C_MessageBoxType aType, std::string aTitle,
		std::string aMessage, C_SDLWindow* aWindow,
		std::vector<C_MessageBoxButton> aButtons, int* aID)
	{
		SDL_Window* win = nullptr;
		unsigned int type;
		std::vector<SDL_MessageBoxButtonData> buttons;

		if (aWindow != nullptr)
			win = aWindow->getHandle();

		//Message box type
		switch (aType)
		{
		case C_MB_INFO:    type = SDL_MESSAGEBOX_INFORMATION; break;
		case C_MB_WARNING: type = SDL_MESSAGEBOX_WARNING;     break;
		case C_MB_ERROR:   type = SDL_MESSAGEBOX_ERROR;       break;
		}

		//Lamda for coverting button data
		auto button = [](C_MessageBoxButton aButton)->unsigned int
		{
			unsigned int flag;
			switch (aButton.flag)
			{
			case C_MB_BUTTON_RETURN_KEY:
				flag = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
				break;
			case C_MB_BUTTON_ESCAPE_KEY:
				flag = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
				break;
			default:
				flag = aButton.flag;
				break;
			}
			return flag;
		};

		for (auto i : aButtons)
		{
			buttons.push_back({ button(i), i.id, i.text });
		}

		SDL_MessageBoxData data = 
		{
			type, win, aTitle.c_str(), aMessage.c_str(),
			buttons.size(), buttons.data(), nullptr
		};

		SDL_ShowMessageBox(&data, aID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_SDLWindow::C_SDLWindow(C_SDLWindowConfig aConfig) :
		mClosed(false),
		mMouseFocus(false),
		mShown(false),
		mMinimized(false),
		mFPSLimit(60),
		mTimeToDraw(1.0 / static_cast<float>(mFPSLimit)),
		mRedrawTime(0.0),
		mFPS(0)
	{
		if (C_SDL_INITED == false)
			initSDL();

		initWindow(aConfig);

		glewExperimental = GL_TRUE;

		if (C_SDL_INITED == false)
		{
			initOpenGL();
			getVer();
			C_SDL_INITED = true;
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Initialize window
	void C_SDLWindow::initWindow(C_SDLWindowConfig aConfig)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

		int flags = SDL_WINDOW_OPENGL;
		if (aConfig.Resizable == true)
			flags |= SDL_WINDOW_RESIZABLE;
		if (aConfig.Fullscreen == true)
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		mWindow = SDL_CreateWindow(aConfig.Title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			aConfig.Width, aConfig.Height, flags);
		mGLC = SDL_GL_CreateContext(mWindow);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Initialize OpenGL
	void C_SDLWindow::initOpenGL()
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		SDL_GL_SetSwapInterval(0);

		C_EnableTextureOpenGL();
		C_EnableBlendOpenGL();
		C_BlendFuncOpenGL(C_OGL_SRC_ALPHA, C_OGL_ONE_MINUS_SRC_ALPHA);
		C_EnableDepthTestOpenGL();
		C_DepthFuncOpenGL(C_OGL_LEQUAL);
		C_EnableCullFaceOpenGL();
		C_CullFaceOpenGL(C_OGL_BACK);
		C_EnableCubemapOpenGL();
		C_EnableMultisampleOpenGL();

		if (glewInit() != GLEW_OK)
			C_Log::fatal("Can't initialize GLEW");
		else
			C_Log::initialization("GLEW initialized");
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Initialize SDL
	void C_SDLWindow::initSDL()
	{
		if (SDL_Init(SDL_INIT_EVERYTHING))
			C_Log::fatal("Can't initialize SDL2");
		else
			C_Log::initialization("SDL2 initialized");
	}
	//////////////////////////////////////////////////////////////////////////////
	//Printing API versions int console
	void C_SDLWindow::getVer()
	{
		SDL_version cVer;
		SDL_version lVer;

		SDL_VERSION(&cVer);
		SDL_GetVersion(&lVer);

		C_Log::initialization("SDL version: %d.%d.%d", cVer.major, cVer.minor, cVer.patch);
		C_Log::initialization("SDL linked version:%d.%d.%d", lVer.major, lVer.minor, lVer.patch);
		C_Log::initialization("OpenGL version: " + C_GetVersionOpenGL());
		C_Log::initialization("OpenGL vendor: " + C_GetVendorOpenGL());
		C_Log::initialization("OpenGL renderer: " + C_GetRendererOpenGL());
		C_Log::initialization("GLSL version: " + C_GetGLSLVersionOpenGL() + "\n");
	}
	//////////////////////////////////////////////////////////////////////////////
	//Poll window events
	void C_SDLWindow::pollEvent(SDL_Event& aEvent)
	{
		mTmpEvent = aEvent;

		if(aEvent.type == SDL_WINDOWEVENT && aEvent.window.windowID == SDL_GetWindowID(mWindow))
		{
			switch (aEvent.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				SDL_HideWindow(mWindow);
				mClosed = true;
				break;
			case SDL_WINDOWEVENT_SHOWN:
				mShown = true;
				break;
			case SDL_WINDOWEVENT_HIDDEN:
				mShown = false;
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				mMinimized = true;
				break;
			case SDL_WINDOWEVENT_MAXIMIZED:
				mMinimized = false;
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
	//Clear window
	void C_SDLWindow::clear(float r, float g, float b, float a)
	{
		SDL_GL_MakeCurrent(mWindow, mGLC);
		C_ClearColorOpenGL(r, g, b, a);
		C_ClearOpenGL(C_OGL_COLOR_BUFFER_BIT | C_OGL_DEPTH_BUFFER_BIT);
		C_ViewportOpenGL(0, 0, getSize().x, getSize().y);
		mDrawTime.reset();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Display window
	void C_SDLWindow::display()
	{
		glFinish();
		SDL_GL_SwapWindow(mWindow);

		float RedrawTime = mDrawTime.elapsed() * 1000;

		int DelayMs = (int)(mTimeToDraw * 1000 - RedrawTime);

		if (DelayMs - 1 > 0)
			SDL_Delay(DelayMs);

		mRedrawTime = mDrawTime.elapsed() * 1000;

		mFPS = (int)(1.0 / (mRedrawTime / 1000));
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Set vertical sync
	void C_SDLWindow::setVerticalSync(bool aV)
	{
		SDL_GL_SetSwapInterval(aV ? 1 : 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set FPS limit
	void C_SDLWindow::setFPSLimit(unsigned aFPSLimit)
	{
		mFPSLimit = aFPSLimit;
		mTimeToDraw = 1.0 / (float)mFPSLimit;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Return window size
	void C_SDLWindow::getSize(int* aX, int* aY)
	{
		SDL_GetWindowSize(mWindow, aX, aY);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return windpow size
	C_Vector2 C_SDLWindow::getSize()
	{
		int x, y;
		SDL_GetWindowSize(mWindow, &x, &y);
		return C_Vector2(x, y);
	}
	//////////////////////////////////////////////////////////////////////////////
	//return window aspect
	float C_SDLWindow::aspect()
	{
		return (float)getSize().x / (float)getSize().y;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return SDL_Window handle
	SDL_Window* C_SDLWindow::getHandle()
	{
		return mWindow;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return mouse-button-press in window
	bool C_SDLWindow::getMouseButton(int aButton)
	{
		//if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(aButton))
		if(mMouseFocus && mKeyFocus)
			return (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(aButton)) ? true : false;
		else
			return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return FPS limit
	unsigned C_SDLWindow::getFPSLimit()
	{
		return mFPSLimit;
	}
	//////////////////////////////////////////////////////////////////////////////
	//return redraw time
	float C_SDLWindow::getRedrawTime()
	{
		return mRedrawTime / 1000;
	}
	/////////////////////////////////////////////////////////////////////////////
	//Return FPS
	int C_SDLWindow::getFPS()
	{
		return mFPS;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Return window open
	bool C_SDLWindow::isOpen()
	{
		return !mClosed;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return key-focus in window
	bool C_SDLWindow::isKeyFocus()
	{
		return mKeyFocus;
	
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return mouse-focus in window
	bool C_SDLWindow::isMouseFocus()
	{
		return mMouseFocus;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return window shown
	bool C_SDLWindow::isShown()
	{
		return mShown;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return window minimized
	bool C_SDLWindow::isMinimized()
	{
		return mMinimized;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_SDLWindow::~C_SDLWindow()
	{

	}

}
