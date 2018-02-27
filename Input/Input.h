/************************************************
*                    Input.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   29.10.2017                  *
*************************************************/

#pragma once

#include <Math/Vector2.h>
#include <Impl/ImplSDL.h>
#include <GUI/IO.h>

#include <SDL.h>
#include <vector>
#include <functional>
#include <utility>

namespace Columbus
{

	enum C_InputBindType
	{
		C_INPUT_BIND_KEY,
		C_INPUT_BIND_KEY_DOWN,
		C_INPUT_BIND_KEY_UP,
	};

	enum E_InputSystemCursor
	{
		E_INPUT_SYSTEM_CURSOR_ARROW,
		E_INPUT_SYSTEM_CURSOR_IBEAM,
		E_INPUT_SYSTEM_CURSOR_WAIT,
		E_INPUT_SYSTEM_CURSOR_CROSSHAIR,
		E_INPUT_SYSTEM_CURSOR_WAITARROW,
		E_INPUT_SYSTEM_CURSOR_SIZENWSE,
		E_INPUT_SYSTEM_CURSOR_SIZENESW,
		E_INPUT_SYSTEM_CURSOR_SIZEWE,
		E_INPUT_SYSTEM_CURSOR_SIZENS,
		E_INPUT_SYSTEM_CURSOR_SIZEALL,
		E_INPUT_SYSTEM_CURSOR_NO,
		E_INPUT_SYSTEM_CURSOR_HAND
	};

	struct C_InputBind
	{
		C_InputBindType type;
		unsigned int key;
		std::function<void()> func;

		C_InputBind(C_InputBindType aType, unsigned int aKey, std::function<void()> aFunc) :
			type(aType), key(aKey), func(aFunc) {}

		void execute()
		{
			func();
		}
	};

	class C_Input
	{
	private:
		uint8_t* mCurrentKeyboardState = nullptr;
		uint8_t* mPreviousKeyboardState = nullptr;
		uint8_t* mKeyboardStateTmp = nullptr;

		int mKeyboardStateNum = 0;

		C_Vector2 mCurrentMousePosition;
		C_Vector2 mPreviousMousePosition;

		bool mMouseEnabled = true;

		std::vector<C_InputBind> mBinds;

		C_SDLWindow* mWindow = nullptr;
		GUI::C_IO* mIO = nullptr;

		void updateIO();
	public:
		C_Input();

		void bindInput(const C_InputBind aBind);

		void setWindow(const C_SDLWindow* aWindow);
		void setIO(const GUI::C_IO* aIO);

		void showMouseCursor(const bool aX);
		void setSystemCursor(const E_InputSystemCursor aID);
		void setColoredCursor(const void* aPixels, const unsigned int aWidth,
			const unsigned int aHeight, const unsigned int aBPP, const C_Vector2 aHot);
		void setMousePos(const C_Vector2 aPos);
		void setMousePosGlobal(const C_Vector2 aPos);

		void update();

		C_Vector2 getMousePosition();
		C_Vector2 getMouseMovement();

		bool getKey(const unsigned int aKey);
		bool getKeyDown(const unsigned int aKey);
		bool getKeyUp(const unsigned int aKey);

		~C_Input();
	};

}
