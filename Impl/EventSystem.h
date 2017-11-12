/************************************************
*              	  EventSystem.h                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <SDL.h>
#include <vector>
#include <cstring>

#include <Impl/ImplSDL.h>

namespace Columbus
{

	/*#ifdef C_SDL
		#define C_BUTTON_LEFT SDL_BUTTON_LEFT
		#define C_BUTTON_RIGHT SDL_BUTTON_RIGHT
		#define C_BUTTON_MIDDLE SDL_BUTTON_MIDDLE

		#define C_KEY_LEFT SDL_SCANCODE_LEFT
		#define C_KEY_RIGHT SDL_SCANCODE_RIGHT
		#define C_KEY_UP SDL_SCANCODE_UP
		#define C_KEY_DOWN SDL_SCANCODE_DOWN
		#define C_KEY_A SDL_SCANCODE_A
		#define C_KEY_B SDL_SCANCODE_B
		#define C_KEY_C SDL_SCANCODE_C
		#define C_KEY_D SDL_SCANCODE_D
		#define C_KEY_E SDL_SCANCODE_E
		#define C_KEY_F SDL_SCANCODE_F
		#define C_KEY_G SDL_SCANCODE_G
		#define C_KEY_H SDL_SCANCODE_H
		#define C_KEY_I SDL_SCANCODE_I
		#define C_KEY_J SDL_SCANCODE_J
		#define C_KEY_K SDL_SCANCODE_K
		#define C_KEY_L SDL_SCANCODE_L
		#define C_KEY_M SDL_SCANCODE_M
		#define C_KEY_N SDL_SCANCODE_N
		#define C_KEY_O SDL_SCANCODE_O
		#define C_KEY_P SDL_SCANCODE_P
		#define C_KEY_Q SDL_SCANCODE_Q
		#define C_KEY_R SDL_SCANCODE_R
		#define C_KEY_S SDL_SCANCODE_S
		#define C_KEY_T SDL_SCANCODE_T
		#define C_KEY_U SDL_SCANCODE_U
		#define C_KEY_V SDL_SCANCODE_V
		#define C_KEY_W SDL_SCANCODE_W
		#define C_KEY_X SDL_SCANCODE_X
		#define C_KEY_Y SDL_SCANCODE_Y
		#define C_KEY_Z SDL_SCANCODE_Z

		#define C_KEY_APOSTROPHE SDL_SCANCODE_APOSTROPHE
		#define C_KEY_BACKSPACE SDL_SCANCODE_BACKSPACE
		#define C_KEY_CAPSLOCK SDL_SCANCODE_CAPSLOCK
		#define C_KEY_COMMA SDL_SCANCODE_COMMA
		#define C_KEY_DELETE SDL_SCANCODE_DELETE
		#define C_KEY_END SDL_SCANCODE_END
		#define C_KEY_EQUALS SDL_SCANCODE_EQUALS
		#define C_KEY_ESCAPE SDL_SCANCODE_ESCAPE

		#define C_KEY_F1 SDL_SCANCODE_F1
		#define C_KEY_F10 SDL_SCANCODE_F10
		#define C_KEY_F11 SDL_SCANCODE_F11
		#define C_KEY_F12 SDL_SCANCODE_F12
		#define C_KEY_F13 SDL_SCANCODE_F13
		#define C_KEY_F14 SDL_SCANCODE_F14
		#define C_KEY_F15 SDL_SCANCODE_F15
		#define C_KEY_F16 SDL_SCANCODE_F16
		#define C_KEY_F17 SDL_SCANCODE_F17
		#define C_KEY_F18 SDL_SCANCODE_F18
		#define C_KEY_F19 SDL_SCANCODE_F19
		#define C_KEY_F2 SDL_SCANCODE_F2
		#define C_KEY_F20 SDL_SCANCODE_F20
		#define C_KEY_F21 SDL_SCANCODE_F21
		#define C_KEY_F22 SDL_SCANCODE_F22
		#define C_KEY_F23 SDL_SCANCODE_F23
		#define C_KEY_F24 SDL_SCANCODE_F24
		#define C_KEY_F25 SDL_SCANCODE_F25
		#define C_KEY_F26 SDL_SCANCODE_F26
		#define C_KEY_F27 SDL_SCANCODE_F27
		#define C_KEY_F28 SDL_SCANCODE_F28
		#define C_KEY_F29 SDL_SCANCODE_F29
		#define C_KEY_F3 SDL_SCANCODE_F3
		#define C_KEY_F4 SDL_SCANCODE_F4
		#define C_KEY_F5 SDL_SCANCODE_F5
		#define C_KEY_F6 SDL_SCANCODE_F6
		#define C_KEY_F7 SDL_SCANCODE_F7
		#define C_KEY_F8 SDL_SCANCODE_F8
		#define C_KEY_F9 SDL_SCANCODE_F9

		#define C_KEY_INSERT SDL_SCANCODE_INSERT
		#define C_KEYPAD_0 SDL_SCANCODE_0
		#define C_KEYPAD_00 SDL_SCANCODE_00
		#define C_KEYPAD_000 SDL_SCANCODE_000
		#define C_KEYPAD_1 SDL_SCANCODE_1
		#define C_KEYPAD_2 SDL_SCANCODE_2
		#define C_KEYPAD_3 SDL_SCANCODE_3
		#define C_KEYPAD_4 SDL_SCANCODE_4
		#define C_KEYPAD_5 SDL_SCANCODE_5
		#define C_KEYPAD_6 SDL_SCANCODE_6
		#define C_KEYPAD_7 SDL_SCANCODE_7
		#define C_KEYPAD_8 SDL_SCANCODE_8
		#define C_KEYPAD_9 SDL_SCANCODE_9

		#define C_KEY_LALT SDL_SCNACODE_LALT
		#define C_KEY_LCTRL SDL_SCNACODE_LCTRL
		#define C_KEY_LGUI SDL_SCANCODE_LGUI
		#define C_KEY_LSHIFT SDL_SCANCODE_LSHIFT
		#define C_KEY_LEFTBRACKET SDL_SCANCODE_LEFTBRACKET
		#define C_KEY_MINUS SDL_SCANCODE_MINUS
		#define C_KEY_NUMLOCK SDL_SCANCODE_NUMLOCK
		#define C_KEY_PAGEDOWN SDL_SCANCODE_PAGEDOWN
		#define C_KEY_PAGEUP SDL_SCANCODE_PAGEUP
		#define C_KEY_PAUSE SDL_SCANCODE_PAUSE
		#define C_KEY_PERIOD SDL_SCANCODE_PERIOD
		#define C_KEY_PRINTSCREEN SDL_SCANCODE_PRINTSCREEN
		#define C_KEY_RALT SDL_SCANCODE_RALT
		#define C_KEY_RCTRL SDL_SCANCODE_RCTRL
		#define C_KEY_RETURN SDL_SCANCODE_RETURN
		#define C_KEY_RGUI SDL_SCANCODE_RGUI
		#define C_KEY_RIGHTBRACKET SDL_SCANCODE_RIGHTBRACKET
		#define C_KEY_RSHIFT SDL_SCANCODE_RSHIFT
		#define C_KEY_SCROLLLOCK SDL_SCANCODE_SCROLLLOCK
		#define C_KEY_SEMICOLON SDL_SCANCODE_SEMICOLON
		#define C_KEY_SLASH SDL_SCANCODE_SLASH
		#define C_KEY_SPACE SDL_SCANCODE_SPACE
		#define C_KEY_TAB SDL_SCANCODE_TAB
	#endif*/


	/*class C_EventSystem
	{
	private:
		SDL_Event mEvent;
		Uint8* keys;
		SDL_Keycode key;
		char* text;
		bool mouseLeft = false;
		bool mouseRight = false;
		bool mouseMiddle = false;
		bool mouseWheelLeft = false;
		bool mouseWheelRight = false;
		bool mouseWheelUp = false;
		bool mouseWheelDown = false;
	public:
		C_EventSystem();

		void update();

		void mouseCoords(int* aX, int* aY);

		void globalMouseCoords(int* aX, int* aY);

		void mouseRel(int* aX, int *aY);

		bool isMouseButton(int aButton);

		bool isMouseButtonClick(int aButton, int aClicks, int *aX, int* aY);

		bool isKey(int aKey);

		bool isKeyDown(int aKey);

		bool isKeyUp(int aKey);

		bool isText(char aText);

		bool isExit();

		char getText();

		int getWheelX();

		int getWheelY();

		~C_EventSystem();
	};*/

	class C_EventSystem
	{
	private:
		SDL_Event mEvent;
		std::vector<C_SDLWindow*> mWindows;

		bool mQuit = false;
	public:
		//Constructor
		C_EventSystem();
		//Poll all events
		void pollEvents();
		//Add window
		void addWindow(C_SDLWindow* aWindow);
		//Return running
		bool isRun();
		//Destructor
		~C_EventSystem();
	};

}
