/************************************************
*              		 ImplSDL.cpp                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Impl/ImplSDL.h>

namespace C
{

	static bool C_SDL_INITED = false;

	#define C_BUTTON_LEFT SDL_BUTTON_LEFT
	#define C_BUTTON_MIDDLE SDL_BUTTON_MIDDLE
	#define C_BUTTON_RIGHT SDL_BUTTON_RIGHT
	#define C_BUTTON_X1 SDL_BUTTON_X1
	#define C_BUTTON_X2 SDL_BUTTON_X2
	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_SDLWindow::C_SDLWindow(int aW, int aH, const char* aTitle)
	{
		if(C_SDL_INITED == false)
		{
			if(SDL_Init(SDL_INIT_EVERYTHING))
			{
				C_FatalError("Can't initialize SDL2");
			} else
			{
				C_Initialization("SDL2 initialized");
			}
		}
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

		mWindow = SDL_CreateWindow(aTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, aW, aH, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		mGLC = SDL_GL_CreateContext(mWindow);
		glewExperimental = GL_TRUE;

		if(C_SDL_INITED == false)
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
   		glEnable(GL_TEXTURE_2D);
   		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_MULTISAMPLE);

		if(glewInit() != GLEW_OK)
		{
			C_FatalError("Can't initialize GLEW");
		} else
		{
			C_Initialization("GLEW initialized");
		}

		SDL_version cVer;
		SDL_version lVer;

		SDL_VERSION(&cVer);
		SDL_GetVersion(&lVer);

		C_Initialization("SDL version: %d.%d.%d", cVer.major, cVer.minor, cVer.patch);
		C_Initialization("SDL linked version:%d.%d.%d", lVer.major, lVer.minor, lVer.patch);
		C_Initialization("OpenGL version: %s", glGetString(GL_VERSION));
		C_Initialization("OpenGL vendor: %s", glGetString(GL_VENDOR));
		C_Initialization("OpenGL renderer: %s", glGetString(GL_RENDERER));
		C_Initialization("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

		C_SDL_INITED = true;

		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 2
	C_SDLWindow::C_SDLWindow(C_SDLWindowConfig aConfig)
	{
		if (C_SDL_INITED == false)
		{
			if (SDL_Init(SDL_INIT_EVERYTHING))
			{
				C_FatalError("Can't initialize SDL2");
			}
			else
			{
				C_Initialization("SDL2 initialized");
			}
		}
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

		int flags = SDL_WINDOW_OPENGL;
		if (aConfig.Resizable == true)
			flags |= SDL_WINDOW_RESIZABLE;
		if (aConfig.Fullscreen == true)
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		mWindow = SDL_CreateWindow(aConfig.Title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, aConfig.Width, aConfig.Height, flags);
		mGLC = SDL_GL_CreateContext(mWindow);
		glewExperimental = GL_TRUE;

		if (C_SDL_INITED == false)
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

			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			glEnable(GL_TEXTURE_CUBE_MAP_ARB);

			glEnable(GL_MULTISAMPLE);

			if (glewInit() != GLEW_OK)
			{
				C_FatalError("Can't initialize GLEW");
			}
			else
			{
				C_Initialization("SDL2 initialized");
			}

			SDL_version cVer;
			SDL_version lVer;

			SDL_VERSION(&cVer);
			SDL_GetVersion(&lVer);

			C_Initialization("SDL version: %d.%d.%d", cVer.major, cVer.minor, cVer.patch);
			C_Initialization("SDL linked version:%d.%d.%d", lVer.major, lVer.minor, lVer.patch);
			C_Initialization("OpenGL version: %s", glGetString(GL_VERSION));
			C_Initialization("OpenGL vendor: %s", glGetString(GL_VENDOR));
			C_Initialization("OpenGL renderer: %s", glGetString(GL_RENDERER));
			C_Initialization("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

			C_SDL_INITED = true;

		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Clear input
	void C_SDLWindow::SYS_CLEAR_INPUT()
	{
		for (size_t i = 0; i < 256; i++)
		{
			keyup[i] = false;
			keydown[i] = false;
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Poll window events
	void C_SDLWindow::pollEvent(SDL_Event& aEvent)
	{
		mTmpEvent = aEvent;

		if(aEvent.type == SDL_WINDOWEVENT && aEvent.window.windowID == SDL_GetWindowID(mWindow))
		{
			if(aEvent.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				SDL_HideWindow(mWindow);
				mClosed = true;
			}

			if(aEvent.window.event == SDL_WINDOWEVENT_SHOWN)
			{
				shown = true;
			}

			if(aEvent.window.event == SDL_WINDOWEVENT_HIDDEN)
			{
				shown = false;
			}

			if(aEvent.window.event == SDL_WINDOWEVENT_MINIMIZED)
			{
				minimized = true;
			}

			if(aEvent.window.event == SDL_WINDOWEVENT_MAXIMIZED)
			{
				minimized = false;
			}

			if(aEvent.window.event == SDL_WINDOWEVENT_ENTER)
			{
				mouseFocus = true;
			}

			if(aEvent.window.event == SDL_WINDOWEVENT_LEAVE)
			{
				mouseFocus = false;
			}

			if(aEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
			{
				keyFocus = true;
			}

			if(aEvent.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
			{
				keyFocus = false;
			}
		}

		if (1)
		{
			if (aEvent.type == SDL_KEYUP)
			{
				keyup[aEvent.key.keysym.scancode] = true;
			}

			if (aEvent.type == SDL_KEYDOWN)
			{
				keydown[aEvent.key.keysym.scancode] = true;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Clear window
	void C_SDLWindow::clear(float r, float g, float b, float a)
	{
		SDL_GL_MakeCurrent(mWindow, mGLC);
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, getSize().x, getSize().y);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Display window
	void C_SDLWindow::display()
	{
		SDL_GL_SwapWindow(mWindow);
		//C_TimeUpdate();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return window open
	bool C_SDLWindow::isOpen()
	{
		return !mClosed;
	}
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
	//Return mouse position
	void C_SDLWindow::getMousePos(int* aX, int* aY)
	{
		SDL_GetMouseState(aX, aY);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return mouse position
	C_Vector2 C_SDLWindow::getMousePos()
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		return C_Vector2(x, y);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return global mouse position
	void C_SDLWindow::getMousePosGlobal(int* aX, int* aY)
	{
		SDL_GetGlobalMouseState(aX, aY);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return global mouse position
	C_Vector2 C_SDLWindow::getMousePosGlobal()
	{
		int x, y;
		SDL_GetGlobalMouseState(&x, &y);
		return C_Vector2(x, y);
	}
	//////////////////////////////////////////////////////////////////////////////
	//return window aspect
	float C_SDLWindow::aspect()
	{
		return (float)getSize().x / (float)getSize().y;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return key-press in window
	bool C_SDLWindow::getKey(int aKey)
	{
		if(keyFocus)
		{
			keys = (Uint8*)SDL_GetKeyboardState(NULL);
			return keys[aKey];
		}
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return key-down in window
	bool C_SDLWindow::getKeyDown(int aKey)
	{
		if (keyFocus)
		{
			return keydown[aKey];
		}

		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return key-up in window
	bool C_SDLWindow::getKeyUp(int aKey)
	{
		if (keyFocus)
		{
			return keyup[aKey];
		}

		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return mouse-button-press in window
	bool C_SDLWindow::getMouseButton(int aButton)
	{
		//if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(aButton))
		if(mouseFocus && keyFocus)
			return (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(aButton)) ? true : false;
		else
			return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_SDLWindow::~C_SDLWindow()
	{

	}

}
