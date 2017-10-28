/************************************************
*                   ImplSDL.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <Math/Vector2.h>
#include <cmath>

#include <SDL.h>

namespace C
{

  class C_Input
  {
  private:
    uint8_t* mCurrentKeyboardState;
    uint8_t* mPreviousKeyboardState;
    uint8_t* mKeyboardState0;
    uint8_t* mKeyboardState1;

    bool mKeyboardState0Active;

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

    ~C_Input();
  };

}
