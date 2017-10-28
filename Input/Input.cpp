#include <Input/Input.h>

namespace C
{

  C_Input::C_Input() :
  mCurrentKeyboardState(nullptr),
  mPreviousKeyboardState(nullptr),
  mKeyboardState0(nullptr),
  mKeyboardState1(nullptr)
  {

  }
  //////////////////////////////////////////////////////////////////////////////
  bool C_Input::init()
  {
    mKeyboardState0 = new uint8_t[256];
    mKeyboardState1 = new uint8_t[256];

    mKeyboardState0 = (uint8_t*)SDL_GetKeyboardState(NULL);
    mKeyboardState1 = (uint8_t*)SDL_GetKeyboardState(NULL);

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
    if (mKeyboardState0Active)
    {
      mKeyboardState1 = (uint8_t*)SDL_GetKeyboardState(NULL);
      mPreviousKeyboardState = mKeyboardState0;
      mCurrentKeyboardState = mKeyboardState1;
    } else
    {
      mKeyboardState0 = (uint8_t*)SDL_GetKeyboardState(NULL);
      mPreviousKeyboardState = mKeyboardState1;
      mCurrentKeyboardState = mKeyboardState0;
    }

    mKeyboardState0Active = !mKeyboardState0Active;

    mPreviousMousePosition = mCurrentMousePosition;

    int mx, my;
    SDL_GetMouseState(&mx, &my);
    mCurrentMousePosition.x = mx;
    mCurrentMousePosition.y = my;
  }
  //////////////////////////////////////////////////////////////////////////////
  C_Vector2 C_Input::getMousePosition()
  {
    return mCurrentMousePosition;
  }
  //////////////////////////////////////////////////////////////////////////////
  C_Vector2 C_Input::getMouseMovement()
  {
    C_Vector2 delta;
    delta.x = mCurrentMousePosition.x - mPreviousMousePosition.x;
    delta.y = mCurrentMousePosition.y - mPreviousMousePosition.y;

    return delta;
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_Input::showMouseCursor(bool aX)
  {
    SDL_ShowCursor(aX ? SDL_ENABLE : SDL_DISABLE);
  }
  //////////////////////////////////////////////////////////////////////////////
  C_Input::~C_Input()
  {

  }

}
