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

	struct SDLWindowConfig
	{
		bool Resizable = false;
		bool Fullscreen = false;
		int Width = 640;
		int Height = 480;
		std::string Title = "Columbus Engine";
	};

	class SDLWindow
	{
	private:
		SDL_Window* mWindow;
		SDL_GLContext mGLC;

		bool mClosed = false;

		bool mKeyFocus = false;
		bool mMouseFocus = false;
		bool mShown = false;
		bool mMinimized = false;

		Timer mDrawTime;
		unsigned mFPSLimit = 60;
		float mTimeToDraw = 1.0f / static_cast<float>(mFPSLimit);
		float mRedrawTime = 0.0;
		int mFPS = 0;

		SDL_Event mTmpEvent;

		void initSDL();
		void initWindow(SDLWindowConfig aConfig);
		void initOpenGL();
		void getVer();
	public:
		SDLWindow(SDLWindowConfig aConfig);

		void pollEvent(SDL_Event& aEvent);
		void clear(float r, float g, float b, float a);
		void display();

		void setVerticalSync(bool aV);
		void setFPSLimit(unsigned aFPSLimit);

		void getSize(int* aX, int* aY);
		Vector2 getSize();
		float aspect();
		SDL_Window* getHandle();
		bool getMouseButton(int aButton);
		unsigned getFPSLimit();
		float getRedrawTime();
		int getFPS();
		
		bool isOpen();
		bool isKeyFocus();
		bool isMouseFocus();
		bool isShown();
		bool isMinimized();
		
		~SDLWindow();
	};

}





