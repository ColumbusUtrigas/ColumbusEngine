/************************************************
*                    Input.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   29.10.2017                  *
*************************************************/

#pragma once

#include <Math/Vector2.h>
#include <Impl/ImplSDL.h>

#include <SDL.h>
#include <vector>
#include <functional>

namespace C
{

	enum C_InputBindType
	{
		C_INPUT_BIND_KEY,
		C_INPUT_BIND_KEY_DOWN,
		C_INPUT_BIND_KEY_UP,
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
		uint8_t* mCurrentKeyboardState;
		uint8_t* mPreviousKeyboardState;
		uint8_t* mKeyboardStateTmp;

		int mKeyboardStateNum;

		C_Vector2 mCurrentMousePosition;
		C_Vector2 mPreviousMousePosition;

		std::vector<C_InputBind> mBinds;

		C_SDLWindow* mWindow;
	public:
		C_Input();
		bool init();

		void bindInput(C_InputBind& aBind);

		bool getKey(unsigned int aKey);
		bool getKeyDown(unsigned int aKey);
		bool getKeyUp(unsigned int aKey);

		void setWindow(C_SDLWindow* aWindow);

		void update();

		C_Vector2 getMousePosition();
		C_Vector2 getMouseMovement();

		void showMouseCursor(bool aX);
		void setMousePos(C_Vector2 aPos);
		void setMousePosGlobal(C_Vector2 aPos);

		~C_Input();
	};

}
