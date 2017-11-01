/************************************************
*                   ImplSDL.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   31.10.2017                  *
*************************************************/

#pragma once

#include <cstdio>
#include <SDL.h>

#include <RenderAPI/APIOpenGL.h>

#include <Math/Vector2.h>
#include <System/Console.h>
#include <System/Timer.h>

namespace C
{

	enum C_MessageBoxType;
	enum C_MessageBoxButtonFlag;
	struct C_MessageBoxButton;
	struct C_SDLWindowConfig;
	class C_SDLWindow;

	//Show simple message box, modal of window
	void C_ShowMessageBox(C_MessageBoxType aType, std::string aTitle,
		std::string aMessage, C_SDLWindow* aWindow);
	//Show complex message box, modal of window
	void C_ShowMessageBox(C_MessageBoxType aType, std::string aTitle,
		std::string aMessage, C_SDLWindow* aWindow,
		std::vector<C_MessageBoxButton> aButtons, int* aID);

	enum C_MessageBoxType
	{
		C_MB_INFO,
		C_MB_WARNING,
		C_MB_ERROR
	};

	enum C_MessageBoxButtonFlag
	{
		C_MB_BUTTON_RETURN_KEY,
		C_MB_BUTTON_ESCAPE_KEY,
	};

	struct C_MessageBoxButton
	{
		unsigned int flag;
		int id;
		const char* text;
	};

	struct C_SDLWindowConfig
	{
		bool Resizable = false;
		bool Fullscreen = false;
		int Width = 640;
		int Height = 480;
		std::string Title = "Columbus Engine";
	};

	class C_SDLWindow
	{
	private:
		SDL_Window* mWindow;
		SDL_GLContext mGLC;

		bool mClosed = false;

		bool mKeyFocus = false;
		bool mMouseFocus = false;
		bool mShown = false;
		bool mMinimized = false;

		C_Timer mDrawTime;
		unsigned mFPSLimit = 60;
		float mTimeToDraw = 1.0 / static_cast<float>(mFPSLimit);
		float mRedrawTime = 0.0;
		int mFPS = 0;

		SDL_Event mTmpEvent;

		//Initialize SDL
		void initSDL();
		//Initialize window
		void initWindow(C_SDLWindowConfig aConfig);
		//Initialize OpenGL
		void initOpenGL();
		//Printing API versions in console
		void getVer();
	public:
		//Constructor
		C_SDLWindow(C_SDLWindowConfig aConfig);
		//Poll window events
		void pollEvent(SDL_Event& aEvent);
		//Clear window
		void clear(float r, float g, float b, float a);
		//Display window
		void display();

		//Set vertical sync
		void setVerticalSync(bool aV);
		//Set FPS limit
		void setFPSLimit(unsigned aFPSLimit);

		//Return window size
		void getSize(int* aX, int* aY);
		//Return window size
		C_Vector2 getSize();
		//Return window aspect
		float aspect();
		//Return SDL_Window handle
		SDL_Window* getHandle();
		//Return mouse-button-press in window
		bool getMouseButton(int aButton);
		//Return FPS limit
		unsigned getFPSLimit();
		//Return redraw time
		float getRedrawTime();
		//Return FPS;
		int getFPS();
		
		//Return window open
		bool isOpen();
		//Return key-focus in window
		bool isKeyFocus();
		//Return mouse-focus in window
		bool isMouseFocus();
		//Return window shown
		bool isShown();
		//Return window minimized
		bool isMinimized();
		
		//Destructor
		~C_SDLWindow();
	};

}





