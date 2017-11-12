/************************************************
*                   ImplSDL.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   31.10.2017                  *
*************************************************/

#pragma once

#include <cstdio>
#include <SDL.h>

#include <RenderAPI/APIOpenGL.h>

#include <Math/Vector2.h>
#include <System/Log.h>
#include <System/Timer.h>

namespace Columbus
{
	
	struct C_SDLWindowConfig;
	class C_SDLWindow;

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





