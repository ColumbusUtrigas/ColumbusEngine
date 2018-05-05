/************************************************
*                   Input.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   29.10.2017                  *
*************************************************/

#include <Input/Input.h>

namespace Columbus
{

	Input::Input() :
		mCurrentKeyboardState(nullptr),
		mPreviousKeyboardState(nullptr),
		mKeyboardStateTmp(nullptr),
		mKeyboardStateNum(0),
		mMouseEnabled(true),
		mWindow(nullptr),
		mIO(nullptr)
	{
		mKeyboardStateTmp = (uint8_t*)SDL_GetKeyboardState(&mKeyboardStateNum);
		mPreviousKeyboardState = new uint8_t[mKeyboardStateNum];
		mCurrentKeyboardState = new uint8_t[mKeyboardStateNum];
	}
	
	void Input::updateIO()
	{
		if (mIO == nullptr || mWindow == nullptr) return;

		mIO->mouse.coords = mCurrentMousePosition;
		mIO->mouse.enabled = mMouseEnabled;
		mIO->screen.aspect = mWindow->getAspect();
		mIO->screen.size = mWindow->getSize();
	}
	
	void Input::bindInput(const InputBind aBind)
	{
		mBinds.push_back(std::move(aBind));
	}
	
	void Input::setWindow(const Window* aWindow)
	{
		if (aWindow == nullptr) return;
		if (aWindow->getType() == "WindowOpenGLSDL")
		{
			mWindow = const_cast<Window*>(aWindow);
		}
	}
	void Input::setIO(const GUI::IO* aIO)
	{
		mIO = const_cast<GUI::IO*>(aIO);
	}
	
	void Input::showMouseCursor(const bool aX)
	{
		mMouseEnabled = static_cast<bool>(aX);
		SDL_ShowCursor(aX ? SDL_ENABLE : SDL_DISABLE);
	}
	
	void Input::SetSystemCursor(SystemCursor Cursor)
	{
		SDL_SystemCursor id = SDL_SYSTEM_CURSOR_NO;

		switch (Cursor)
		{
		case SystemCursor::Arrow: id = SDL_SYSTEM_CURSOR_ARROW; break;
		case SystemCursor::IBeam: id = SDL_SYSTEM_CURSOR_IBEAM; break;
		case SystemCursor::Wait: id = SDL_SYSTEM_CURSOR_WAIT; break;
		case SystemCursor::Crosshair: id = SDL_SYSTEM_CURSOR_CROSSHAIR; break;
		case SystemCursor::WaitArrow: id = SDL_SYSTEM_CURSOR_WAITARROW; break;
		case SystemCursor::SizeNWSE: id = SDL_SYSTEM_CURSOR_SIZENWSE; break;
		case SystemCursor::SizeNESW: id = SDL_SYSTEM_CURSOR_SIZENESW; break;
		case SystemCursor::SizeWE: id = SDL_SYSTEM_CURSOR_SIZEWE; break;
		case SystemCursor::SizeNS: id = SDL_SYSTEM_CURSOR_SIZENS; break;
		case SystemCursor::SizeAll: id = SDL_SYSTEM_CURSOR_SIZEALL; break;
		case SystemCursor::No: id = SDL_SYSTEM_CURSOR_NO; break;
		case SystemCursor::Hand: id = SDL_SYSTEM_CURSOR_HAND; break;
		default: id = SDL_SYSTEM_CURSOR_NO; break;
		}

		SDL_Cursor* cursor = SDL_CreateSystemCursor(id);
		SDL_SetCursor(cursor);
	}

	void Input::SetColoredCursor(const void* aPixels, const unsigned int aW,
			const unsigned int aH, const unsigned int aBPP, const Vector2 aHot)
	{
		Uint32 rmask, gmask, bmask, amask;
		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			int shift = (aBPP == 4) ? 8 : 0;
			rmask = 0xff000000 >> shift;
			gmask = 0x00ff0000 >> shift;
			bmask = 0x0000ff00 >> shift;
			amask = 0x000000ff >> shift;
		#else
			rmask = 0x000000ff;
			gmask = 0x0000ff00;
			bmask = 0x00ff0000;
			amask = (aBPP == 3) ? 0 : 0xff000000;
		#endif

		SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(const_cast<void*>(aPixels), aW, aH, aBPP * 8, aW * aBPP, rmask, gmask, bmask, amask);
		SDL_Cursor* cursor = SDL_CreateColorCursor(surf, static_cast<int>(aHot.X), static_cast<int>(aHot.Y));
		SDL_SetCursor(cursor);
	}
	
	void Input::setMousePos(const Vector2 aPos)
	{
		if (mWindow == nullptr) return;

		int x = Math::TruncToInt(aPos.X);
		int y = Math::TruncToInt(aPos.Y);
		SDL_WarpMouseInWindow((static_cast<WindowOpenGLSDL*>(mWindow))->getHandle(), x, y);
		mCurrentMousePosition = aPos;
		mPreviousMousePosition = aPos;
	}
	
	void Input::setMousePosGlobal(const Vector2 aPos)
	{
		SDL_WarpMouseGlobal(static_cast<int>(aPos.X), static_cast<int>(aPos.Y));
	}
	
	void Input::update()
	{
		SDL_PumpEvents();

		for (int i = 0; i < mKeyboardStateNum; i++)
			mPreviousKeyboardState[i] = mCurrentKeyboardState[i];

		for (int i = 0; i < mKeyboardStateNum; i++)
			mCurrentKeyboardState[i] = mKeyboardStateTmp[i];

		mPreviousMousePosition = mCurrentMousePosition;

		int mx, my;
		SDL_GetMouseState(&mx, &my);
		mCurrentMousePosition.X = static_cast<float>(mx);
		mCurrentMousePosition.Y = static_cast<float>(my);

		updateIO();

		for (auto& Bind : mBinds)
		{
			switch (Bind.Type)
			{
			case InputBindType::Key:
				if (getKey(Bind.Key))
				{
					Bind.Execute();
				}
				break;
			case InputBindType::KeyDown:
				if (getKeyDown(Bind.Key))
				{
					Bind.Execute();
				}
				break;
			case InputBindType::KeyUp:
				if (getKeyUp(Bind.Key))
				{
					Bind.Execute();
				}
				break;
			}
		}
	}
	
	Vector2 Input::getMousePosition()
	{
		return mCurrentMousePosition;
	}
	
	Vector2 Input::getMouseMovement()
	{
		Vector2 mouseDelta;
		int preX = static_cast<int>(mPreviousMousePosition.X);
		int preY = static_cast<int>(mPreviousMousePosition.Y);
		int curX = static_cast<int>(mCurrentMousePosition.X);
		int curY = static_cast<int>(mCurrentMousePosition.Y);
		mouseDelta.X = static_cast<float>(curX - preX);
		mouseDelta.Y = static_cast<float>(curY - preY);
		return mouseDelta;
	}
	
	bool Input::getKey(const unsigned int aKey)
	{
		if (mWindow != nullptr)
			if (mWindow->isKeyFocus() == false)
				return false;

		return ((mPreviousKeyboardState[aKey] != 0x00) && (mCurrentKeyboardState[aKey] != 0x00));
	}
	
	bool Input::getKeyDown(const unsigned int aKey)
	{
		if (mWindow != nullptr)
			if (mWindow->isKeyFocus() == false)
				return false;

		return ((mPreviousKeyboardState[aKey] == 0x00) && (mCurrentKeyboardState[aKey] != 0x00));
	}
	
	bool Input::getKeyUp(const unsigned int aKey)
	{
		if (mWindow != nullptr)
			if (mWindow->isKeyFocus() == false)
				return false;

		return ((mPreviousKeyboardState[aKey] != 0x00) && (mCurrentKeyboardState[aKey] == 0x00));
	}
	
	Input::~Input()
	{

	}

}



