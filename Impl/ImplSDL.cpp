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
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	SDLWindow::SDLWindow(SDLWindowConfig aConfig) :
		mClosed(false),
		mMouseFocus(false),
		mShown(false),
		mMinimized(false),
		mFPSLimit(60),
		mTimeToDraw(1.0f / static_cast<float>(mFPSLimit)),
		mRedrawTime(0.0f),
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
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void SDLWindow::initWindow(SDLWindowConfig aConfig)
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
	void SDLWindow::initOpenGL()
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
		C_EnableAlphaTestOpenGL();

		if (glewInit() != GLEW_OK)
			Log::fatal("Can't initialize GLEW");
		else
			Log::initialization("GLEW initialized");
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void SDLWindow::initSDL()
	{
		if (SDL_Init(SDL_INIT_EVERYTHING))
			Log::fatal("Can't initialize SDL2");
		else
			Log::initialization("SDL2 initialized");
	}
	//////////////////////////////////////////////////////////////////////////////
	void SDLWindow::getVer()
	{
		SDL_version cVer;
		SDL_version lVer;

		SDL_VERSION(&cVer);
		SDL_GetVersion(&lVer);

		Log::initialization("SDL version: %d.%d.%d", cVer.major, cVer.minor, cVer.patch);
		Log::initialization("SDL linked version: %d.%d.%d", lVer.major, lVer.minor, lVer.patch);
		Log::initialization("OpenGL version: " + C_GetVersionOpenGL());
		Log::initialization("OpenGL vendor: " + C_GetVendorOpenGL());
		Log::initialization("OpenGL renderer: " + C_GetRendererOpenGL());
		Log::initialization("GLSL version: " + C_GetGLSLVersionOpenGL() + "\n");
	}
	//////////////////////////////////////////////////////////////////////////////
	void SDLWindow::pollEvent(SDL_Event& aEvent)
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
	void SDLWindow::clear(float r, float g, float b, float a)
	{
		SDL_GL_MakeCurrent(mWindow, mGLC);
		C_ClearColorOpenGL(r, g, b, a);
		C_ClearOpenGL(C_OGL_COLOR_BUFFER_BIT | C_OGL_DEPTH_BUFFER_BIT);
		C_ViewportOpenGL(0, 0, static_cast<size_t>(getSize().x), static_cast<size_t>(getSize().y));
		mDrawTime.reset();
	}
	//////////////////////////////////////////////////////////////////////////////
	void SDLWindow::display()
	{
		glFinish();
		SDL_GL_SwapWindow(mWindow);

		float RedrawTime = static_cast<float>(mDrawTime.elapsed() * 1000);

		int DelayMs = (int)(mTimeToDraw * 1000 - RedrawTime);
		

		if (DelayMs - 1 > 0)
			SDL_Delay(DelayMs);
		mRedrawTime = static_cast<float>(mDrawTime.elapsed() * 1000);

		mFPS = (int)(1.0 / (mRedrawTime / 1000));
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void SDLWindow::setVerticalSync(bool aV)
	{
		SDL_GL_SetSwapInterval(aV ? 1 : 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	void SDLWindow::setFPSLimit(unsigned aFPSLimit)
	{
		mFPSLimit = aFPSLimit;
		mTimeToDraw = 1.0f / (float)mFPSLimit;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void SDLWindow::getSize(int* aX, int* aY)
	{
		SDL_GetWindowSize(mWindow, aX, aY);
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector2 SDLWindow::getSize()
	{
		int x, y;
		SDL_GetWindowSize(mWindow, &x, &y);
		return Vector2(static_cast<float>(x), static_cast<float>(y));
	}
	//////////////////////////////////////////////////////////////////////////////
	float SDLWindow::aspect()
	{
		return (float)getSize().x / (float)getSize().y;
	}
	//////////////////////////////////////////////////////////////////////////////
	SDL_Window* SDLWindow::getHandle()
	{
		return mWindow;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool SDLWindow::getMouseButton(int aButton)
	{
		//if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(aButton))
		if(mMouseFocus && mKeyFocus)
			return (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(aButton)) ? true : false;
		else
			return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	unsigned SDLWindow::getFPSLimit()
	{
		return mFPSLimit;
	}
	//////////////////////////////////////////////////////////////////////////////
	float SDLWindow::getRedrawTime()
	{
		return mRedrawTime / 1000;
	}
	/////////////////////////////////////////////////////////////////////////////
	int SDLWindow::getFPS()
	{
		return mFPS;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool SDLWindow::isOpen()
	{
		return !mClosed;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool SDLWindow::isKeyFocus()
	{
		return mKeyFocus;
	
	}
	//////////////////////////////////////////////////////////////////////////////
	bool SDLWindow::isMouseFocus()
	{
		return mMouseFocus;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool SDLWindow::isShown()
	{
		return mShown;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool SDLWindow::isMinimized()
	{
		return mMinimized;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	SDLWindow::~SDLWindow()
	{

	}

}




