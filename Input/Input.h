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
#include <Graphics/OpenGL/WindowOpenGLSDL.h>
#include <GUI/IO.h>
#include <Common/Cursor/Cursor.h>

#include <SDL.h>
#include <vector>
#include <functional>
#include <utility>

namespace Columbus
{

	enum class InputBindType
	{
		Key,
		KeyDown,
		KeyUp
	};

	enum class SystemCursor
	{
		Arrow,
		IBeam,
		Wait,
		Crosshair,
		WaitArrow,
		SizeNWSE,
		SizeNESW,
		SizeWE,
		SizeNS,
		SizeAll,
		No,
		Hand
	};

	struct InputBind
	{
		InputBindType Type;
		uint32 Key;
		std::function<void()> Func;

		InputBind(InputBindType InType, uint32 InKey, std::function<void()> InFunc) :
			Type(InType), Key(InKey), Func(InFunc)
		{}

		void Execute()
		{
			Func();
		}
	};

	class Input
	{
	private:
		uint8* mCurrentKeyboardState = nullptr;
		uint8* mPreviousKeyboardState = nullptr;
		uint8* mKeyboardStateTmp = nullptr;

		int mKeyboardStateNum = 0;

		Vector2 mCurrentMousePosition;
		Vector2 mPreviousMousePosition;

		bool mMouseEnabled = true;

		std::vector<InputBind> mBinds;

		Window* mWindow = nullptr;
		GUI::IO* mIO = nullptr;

		void updateIO();
	public:
		Input();

		void bindInput(const InputBind aBind);

		void setWindow(const Window* aWindow);
		void setIO(const GUI::IO* aIO);

		void showMouseCursor(const bool aX);
		void SetCursor(Cursor InCursor);
		void SetSystemCursor(SystemCursor Cursor);
		void SetColoredCursor(const void* aPixels, const unsigned int aWidth,
			const unsigned int aHeight, const unsigned int aBPP, const Vector2 aHot);
		void setMousePos(const Vector2 aPos);
		void setMousePosGlobal(const Vector2 aPos);

		void update();

		Vector2 getMousePosition();
		Vector2 getMouseMovement();

		bool getKey(const unsigned int aKey);
		bool getKeyDown(const unsigned int aKey);
		bool getKeyUp(const unsigned int aKey);

		~Input();
	};

}
