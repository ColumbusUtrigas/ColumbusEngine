/************************************************
*                   ImplSDL.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#ifndef IMPLSDL_H
#define IMPLSDL_H

#include <cstdio>
#include <SDL.h>
#include <GL/glew.h>

#include <Math/Vector2.h>
#include <System/Console.h>

namespace C
{

	struct C_SDLWindowConfig
	{
		bool Resizable = false;
		bool Fullscreen = false;
		int Width = 640;
		int Height = 480;
		char* Title = (char*)"Columbus Engine";
	};

	class C_SDLWindow
	{
	private:
		SDL_Window* mWindow;
		SDL_GLContext mGLC;

		bool mClosed = false;

		bool keyFocus = false;
		bool mouseFocus = false;
		bool shown = false;
		bool minimized = false;

		Uint8* keys;

		bool keydown[256];
		bool keyup[256];

		SDL_Event mTmpEvent;
	public:
		//Constructor
		C_SDLWindow(int aW, int aH, const char* aTitle);
		//Constructor 2
		C_SDLWindow(C_SDLWindowConfig aConfig);
		//Clear input
		void SYS_CLEAR_INPUT();
		//Poll window events
		void pollEvent(SDL_Event& aEvent);
		//Clear window
		void clear(float r, float g, float b, float a);
		//Display window
		void display();
		//Return window size
		void getSize(int* aX, int* aY);
		//Return window size
		C_Vector2 getSize();
		//Return mouse position
		void getMousePos(int* aX, int* aY);
		//Return mouse position
		C_Vector2 getMousePos();
		//Return global mouse position
		void getMousePosGlobal(int* aX, int* aY);
		//Return global mouse position
		C_Vector2 getMousePosGlobal();
		//Return window aspect
		float aspect();
		//Return window open
		bool isOpen();
		//Return key-focus in window
		bool isKeyFocus();
		//return window mouse-focus
		bool isMouseFocus();
		//Return window shown
		bool isShown();
		//Return window minimized
		bool isMinimized();
		//Return key-press in window
		bool getKey(int aKey);
		//Return key-down in window
		bool getKeyDown(int aKey);
		//Return key-up in window
		bool getKeyUp(int aKey);
		//Return mouse-button-press in window
		bool getMouseButton(int aButton);
		//Set vertical sync
		void setVerticalSync(bool aV);
		//Destructor
		~C_SDLWindow();
	};

}

#endif
