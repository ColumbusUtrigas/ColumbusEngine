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

namespace C
{

  class C_Input
  {
  private:
    uint8_t* mCurrentKeyboardState;
    uint8_t* mPreviousKeyboardState;
    uint8_t* mKeyboardStateTmp;

    int mKeyboardStateNum;

    C_Vector2 mCurrentMousePosition;
    C_Vector2 mPreviousMousePosition;
  public:
    C_Input();
    bool init();

    //void bindInput();

    bool isKeyDown(unsigned int aKey);
    bool isKeyUp(unsigned int aKey);
    bool isKeyPressed(unsigned int aKey);
    bool isKeyReleased(unsigned int aKey);

    void update();

    C_Vector2 getMousePosition();
    C_Vector2 getMouseMovement();

    void showMouseCursor(bool aX);
    void setMousePos(C_Vector2 aPos, C_SDLWindow& aWindow);
    void setMousePosGlobal(C_Vector2 aPos);

    ~C_Input();
  };

}
