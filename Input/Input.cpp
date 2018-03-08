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
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void Input::updateIO()
	{
		if (mIO == nullptr || mWindow == nullptr)
			return;

		mIO->mouse.coords = mCurrentMousePosition;
		mIO->mouse.enabled = mMouseEnabled;
		mIO->screen.aspect = mWindow->aspect();
		mIO->screen.size = mWindow->getSize();
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void Input::bindInput(const InputBind aBind)
	{
		mBinds.push_back(std::move(aBind));
	}
	//////////////////////////////////////////////////////////////////////////////
	void Input::setWindow(const SDLWindow* aWindow)
	{
		mWindow = const_cast<SDLWindow*>(aWindow);
	}
	void Input::setIO(const GUI::IO* aIO)
	{
		mIO = const_cast<GUI::IO*>(aIO);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Input::showMouseCursor(const bool aX)
	{
		mMouseEnabled = static_cast<bool>(aX);
		SDL_ShowCursor(aX ? SDL_ENABLE : SDL_DISABLE);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Input::setSystemCursor(const InputSystemCursor aID)
	{
		SDL_SystemCursor id = SDL_SYSTEM_CURSOR_NO;

		switch (aID)
		{
		case E_INPUT_SYSTEM_CURSOR_ARROW: id = SDL_SYSTEM_CURSOR_ARROW; break;
		case E_INPUT_SYSTEM_CURSOR_IBEAM: id = SDL_SYSTEM_CURSOR_IBEAM; break;
		case E_INPUT_SYSTEM_CURSOR_WAIT: id = SDL_SYSTEM_CURSOR_WAIT; break;
		case E_INPUT_SYSTEM_CURSOR_CROSSHAIR: id = SDL_SYSTEM_CURSOR_CROSSHAIR; break;
		case E_INPUT_SYSTEM_CURSOR_WAITARROW: id = SDL_SYSTEM_CURSOR_WAITARROW; break;
		case E_INPUT_SYSTEM_CURSOR_SIZENWSE: id = SDL_SYSTEM_CURSOR_SIZENWSE; break;
		case E_INPUT_SYSTEM_CURSOR_SIZENESW: id = SDL_SYSTEM_CURSOR_SIZENESW; break;
		case E_INPUT_SYSTEM_CURSOR_SIZEWE: id = SDL_SYSTEM_CURSOR_SIZEWE; break;
		case E_INPUT_SYSTEM_CURSOR_SIZENS: id = SDL_SYSTEM_CURSOR_SIZENS; break;
		case E_INPUT_SYSTEM_CURSOR_SIZEALL: id = SDL_SYSTEM_CURSOR_SIZEALL; break;
		case E_INPUT_SYSTEM_CURSOR_NO: id = SDL_SYSTEM_CURSOR_NO; break;
		case E_INPUT_SYSTEM_CURSOR_HAND: id = SDL_SYSTEM_CURSOR_HAND; break;
		default: id = SDL_SYSTEM_CURSOR_NO; break;
		}

		SDL_Cursor* cursor = SDL_CreateSystemCursor(id);
		SDL_SetCursor(cursor);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Input::setColoredCursor(const void* aPixels, const unsigned int aW,
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
		SDL_Cursor* cursor = SDL_CreateColorCursor(surf, static_cast<int>(aHot.x), static_cast<int>(aHot.y));
		SDL_SetCursor(cursor);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Input::setMousePos(const Vector2 aPos)
	{
		if (mWindow == nullptr)
			return;

		int x = static_cast<int>(aPos.x);
		int y = static_cast<int>(aPos.y);
		SDL_WarpMouseInWindow(mWindow->getHandle(), x, y);
		mCurrentMousePosition = aPos;
		mPreviousMousePosition = aPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	void Input::setMousePosGlobal(const Vector2 aPos)
	{
		SDL_WarpMouseGlobal(static_cast<int>(aPos.x), static_cast<int>(aPos.y));
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
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
		mCurrentMousePosition.x = static_cast<float>(mx);
		mCurrentMousePosition.y = static_cast<float>(my);

		updateIO();

		for (auto i : mBinds)
		{
			switch (i.type)
			{
			case E_INPUT_BIND_KEY:
				if (getKey(i.key))
					i.execute();
				break;
			case E_INPUT_BIND_KEY_DOWN:
				if (getKeyDown(i.key))
					i.execute();
				break;
			case E_INPUT_BIND_KEY_UP:
				if (getKeyUp(i.key))
					i.execute();
				break;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Vector2 Input::getMousePosition()
	{
		return mCurrentMousePosition;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector2 Input::getMouseMovement()
	{
		Vector2 mouseDelta;
		int preX = static_cast<int>(mPreviousMousePosition.x);
		int preY = static_cast<int>(mPreviousMousePosition.y);
		int curX = static_cast<int>(mCurrentMousePosition.x);
		int curY = static_cast<int>(mCurrentMousePosition.y);
		mouseDelta.x = static_cast<float>(curX - preX);
		mouseDelta.y = static_cast<float>(curY - preY);
		return mouseDelta;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool Input::getKey(const unsigned int aKey)
	{
		if (mWindow != nullptr)
			if (mWindow->isKeyFocus() == false)
				return false;

		return ((mPreviousKeyboardState[aKey] != 0x00) && (mCurrentKeyboardState[aKey] != 0x00));
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Input::getKeyDown(const unsigned int aKey)
	{
		if (mWindow != nullptr)
			if (mWindow->isKeyFocus() == false)
				return false;

		return ((mPreviousKeyboardState[aKey] == 0x00) && (mCurrentKeyboardState[aKey] != 0x00));
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Input::getKeyUp(const unsigned int aKey)
	{
		if (mWindow != nullptr)
			if (mWindow->isKeyFocus() == false)
				return false;

		return ((mPreviousKeyboardState[aKey] != 0x00) && (mCurrentKeyboardState[aKey] == 0x00));
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Input::~Input()
	{

	}

}
