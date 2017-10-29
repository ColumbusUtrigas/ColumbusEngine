/************************************************
*                   Input.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   29.10.2017                  *
*************************************************/

#include <Input/Input.h>

namespace C
{

  C_Input::C_Input() :
  mCurrentKeyboardState(nullptr),
  mPreviousKeyboardState(nullptr),
  mKeyboardStateTmp(nullptr),
  mKeyboardStateNum(0)
  {

  }
  //////////////////////////////////////////////////////////////////////////////
  bool C_Input::init()
  {
    mKeyboardStateTmp = (uint8_t*)SDL_GetKeyboardState(&mKeyboardStateNum);
    mPreviousKeyboardState = new uint8_t[mKeyboardStateNum];
    mCurrentKeyboardState = new uint8_t[mKeyboardStateNum];

    return true;
  }
  //////////////////////////////////////////////////////////////////////////////
  bool C_Input::isKeyDown(unsigned int aKey)
  {
    return ((mPreviousKeyboardState[aKey] == true) && (mCurrentKeyboardState[aKey] == true));
  }
  //////////////////////////////////////////////////////////////////////////////
  bool C_Input::isKeyUp(unsigned int aKey)
  {
    return ((mPreviousKeyboardState[aKey] == false) && (mCurrentKeyboardState[aKey] == false));
  }
  //////////////////////////////////////////////////////////////////////////////
  bool C_Input::isKeyPressed(unsigned int aKey)
  {
    return ((mPreviousKeyboardState[aKey] == false) && (mCurrentKeyboardState[aKey] == true));
  }
  //////////////////////////////////////////////////////////////////////////////
  bool C_Input::isKeyReleased(unsigned int aKey)
  {
    return ((mPreviousKeyboardState[aKey] == true) && (mCurrentKeyboardState[aKey] == false));
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_Input::update()
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
  }
  //////////////////////////////////////////////////////////////////////////////
  C_Vector2 C_Input::getMousePosition()
  {
    return mCurrentMousePosition;
  }
  //////////////////////////////////////////////////////////////////////////////
  C_Vector2 C_Input::getMouseMovement()
  {
    C_Vector2 mouseDelta;
    int preX = static_cast<int>(mPreviousMousePosition.x);
    int preY = static_cast<int>(mPreviousMousePosition.y);
    int curX = static_cast<int>(mCurrentMousePosition.x);
    int curY = static_cast<int>(mCurrentMousePosition.y);
    mouseDelta.x = static_cast<float>(curX - preX);
    mouseDelta.y = static_cast<float>(curY - preY);
    return mouseDelta;
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_Input::showMouseCursor(bool aX)
  {
    SDL_ShowCursor(aX ? SDL_ENABLE : SDL_DISABLE);
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_Input::setMousePos(C_Vector2 aPos, C_SDLWindow& aWindow)
  {
    int x = static_cast<int>(aPos.x);
    int y = static_cast<int>(aPos.y);
    SDL_WarpMouseInWindow(aWindow.getHandle(), x, y);
    mCurrentMousePosition = aPos;
    mPreviousMousePosition = aPos;
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_Input::setMousePosGlobal(C_Vector2 aPos)
  {
    SDL_WarpMouseGlobal(static_cast<int>(aPos.x), static_cast<int>(aPos.y));
  }
  //////////////////////////////////////////////////////////////////////////////
  C_Input::~C_Input()
  {

  }

}
