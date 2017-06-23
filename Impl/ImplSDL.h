#pragma once

#ifndef IMPLSDL_H
#define IMPLSDL_H

#include <cstdio>
#include <Vector2.h>
#include <SDL.h>
#include <GL/glew.h>

namespace C
{

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
		C_SDLWindow(int aW, int aH, const char* aTitle);

		void SYS_CLEAR_INPUT();

		void pollEvent(SDL_Event& aEvent);

		void clear(float r, float g, float b, float a);

		void display();

		void getSize(int* aX, int* aY);

		C_Vector2 getSize();

		void getMousePos(int* aX, int* aY);

		C_Vector2 getMousePos();

		void getMousePosGlobal(int* aX, int* aY);

		C_Vector2 getMousePosGlobal();

		float aspect();

		bool isOpen();

		bool isKeyFocus();

		bool isMouseFocus();

		bool isShown();

		bool isMinimized();

		bool getKey(int aKey);

		bool getKeyDown(int aKey);

		bool getKeyUp(int aKey);

		bool getMouseButton(int aButton);

		~C_SDLWindow();
	};

}

#endif
