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

		void UpdateIO();
	public:
		Input();

		void BindInput(const InputBind aBind);

		void SetWindow(const Window* aWindow);
		void SetIO(const GUI::IO* aIO);

		void ShowMouseCursor(const bool aX);
		void SetCursor(Cursor InCursor);
		void SetSystemCursor(SystemCursor Cursor);
		void SetColoredCursor(const void* aPixels, const unsigned int aWidth,
			const unsigned int aHeight, const unsigned int aBPP, const Vector2 aHot);
		void SetMousePos(const Vector2 aPos);
		void SetMousePosGlobal(const Vector2 aPos);

		void Update();

		Vector2 GetMousePosition();
		Vector2 GetMouseMovement();

		bool GetKey(const unsigned int aKey);
		bool GetKeyDown(const unsigned int aKey);
		bool GetKeyUp(const unsigned int aKey);

		~Input();
	};

}
