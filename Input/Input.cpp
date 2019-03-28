#include <Input/Input.h>
#include <SDL.h>

namespace Columbus
{

	Input::Input()
	{
		Keyboard.KeyboardState = (uint8*)SDL_GetKeyboardState(&Keyboard.KeysNum);

		for (int Joystick = 0; Joystick < SDL_NumJoysticks(); Joystick++)
		{
			if (SDL_IsGameController(Joystick))
			{
				Gamepad.Internal = SDL_GameControllerOpen(Joystick);
				break;
			}
		}
	}
	
	void Input::ShowMouseCursor(bool Show)
	{
		MouseEnabled = Show;
		SDL_ShowCursor(Show ? SDL_ENABLE : SDL_DISABLE);
	}

	void Input::SetCursor(Cursor InCursor)
	{
		if (InCursor.FramesCount == 0 || InCursor.Frames == nullptr)
		{
			return;
		}

		Uint32 RMask, GMask, BMask, AMask;
		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			RMask = 0xFF000000;
			GMask = 0x00FF0000;
			BMask = 0x0000FF00;
			AMask = 0x000000FF;
		#else
			RMask = 0x000000FF;
			GMask = 0x0000FF00;
			BMask = 0x00FF0000;
			AMask = 0xFF000000;
		#endif

		auto Pixels   = InCursor.Frames[0].PixelData;
		uint32 Width  = InCursor.Frames[0].Width;
		uint32 Height = InCursor.Frames[0].Height;
		uint32 HotX   = InCursor.Frames[0].HotPointX;
		uint32 HotY   = InCursor.Frames[0].HotPointY;

		SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(Pixels, Width, Height, 32, Width * 4, RMask, GMask, BMask, AMask);
		SDL_Cursor* cursor = SDL_CreateColorCursor(surf, HotX, HotY);
		SDL_SetCursor(cursor);
	}
	
	void Input::SetSystemCursor(SystemCursor Cursor)
	{
		SDL_SystemCursor ID = SDL_SYSTEM_CURSOR_NO;

		switch (Cursor)
		{
		case SystemCursor::Arrow:     ID = SDL_SYSTEM_CURSOR_ARROW;     break;
		case SystemCursor::IBeam:     ID = SDL_SYSTEM_CURSOR_IBEAM;     break;
		case SystemCursor::Wait:      ID = SDL_SYSTEM_CURSOR_WAIT;      break;
		case SystemCursor::Crosshair: ID = SDL_SYSTEM_CURSOR_CROSSHAIR; break;
		case SystemCursor::WaitArrow: ID = SDL_SYSTEM_CURSOR_WAITARROW; break;
		case SystemCursor::SizeNWSE:  ID = SDL_SYSTEM_CURSOR_SIZENWSE;  break;
		case SystemCursor::SizeNESW:  ID = SDL_SYSTEM_CURSOR_SIZENESW;  break;
		case SystemCursor::SizeWE:    ID = SDL_SYSTEM_CURSOR_SIZEWE;    break;
		case SystemCursor::SizeNS:    ID = SDL_SYSTEM_CURSOR_SIZENS;    break;
		case SystemCursor::SizeAll:   ID = SDL_SYSTEM_CURSOR_SIZEALL;   break;
		case SystemCursor::No:        ID = SDL_SYSTEM_CURSOR_NO;        break;
		case SystemCursor::Hand:      ID = SDL_SYSTEM_CURSOR_HAND;      break;
		default:                      ID = SDL_SYSTEM_CURSOR_NO;        break;
		}

		static SDL_Cursor* cursor = nullptr;

		if (cursor != nullptr)
		{
			SDL_FreeCursor(cursor);
		}

		cursor = SDL_CreateSystemCursor(ID);
		SDL_SetCursor(cursor);
	}

	void Input::SetColoredCursor(const void* Pixels, uint32 Width, uint32 Height, uint32 BPP, const iVector2& Hot)
	{
		Uint32 RMask, GMask, BMask, AMask;
		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			int Shift = (BPP == 4) ? 8 : 0;
			RMask = 0xFF000000 >> Shift;
			GMask = 0x00FF0000 >> Shift;
			BMask = 0x0000FF00 >> Shift;
			AMask = 0x000000FF >> Shift;
		#else
			RMask = 0x000000FF;
			GMask = 0x0000FF00;
			BMask = 0x00FF0000;
			AMask = (BPP == 3) ? 0 : 0xFF000000;
		#endif

		SDL_Surface* surf = SDL_CreateRGBSurfaceFrom((void*)Pixels, Width, Height, BPP * 8, Width * BPP, RMask, GMask, BMask, AMask);
		SDL_Cursor* cursor = SDL_CreateColorCursor(surf, Hot.X, Hot.Y);
		SDL_SetCursor(cursor);
	}
	
	void Input::WarpMouse(const iVector2& Pos)
	{
		Mouse.CurrentPosition  = Pos;
		Mouse.PreviousPosition = Pos;
	}

	void Input::SetKeyboardFocus(bool Focus)
	{
		KeyboardFocus = Focus;
	}

	void Input::SetMouseFocus(bool Focus)
	{
		MouseFocus = Focus;
	}
	
	void Input::Update()
	{
		SDL_PumpEvents();

		for (int i = 0; i < Keyboard.KeysNum && i < DeviceKeyboard::MaxKeys; i++) Keyboard.Keys[i] = Keyboard.KeyboardState[i] != 0;
		memset(Keyboard.KeysDown, 0, sizeof(Keyboard.KeysDown));
		memset(Keyboard.KeysUp, 0, sizeof(Keyboard.KeysUp));

		for (int i = 0; i < DeviceMouse::MaxButtons; i++) Mouse.Buttons[i].State = SDL_GetMouseState(nullptr, nullptr) & i;
		memset(Mouse.ButtonsDown, 0, sizeof(Mouse.ButtonsDown));
		memset(Mouse.ButtonsUp, 0, sizeof(Mouse.ButtonsUp));

		memset(Gamepad.ButtonsDown, 0, sizeof(Gamepad.ButtonsDown));
		memset(Gamepad.ButtonsUp, 0, sizeof(Gamepad.ButtonsUp));

		Mouse.PreviousPosition = Mouse.CurrentPosition;
		Mouse.Wheel = { 0, 0 };
	}

	void Input::SetKeyDown(uint32 Key)
	{
		Keyboard.KeysDown[Key] = true;
	}

	void Input::SetKeyUp(uint32 Key)
	{
		Keyboard.KeysUp[Key] = true;
	}

	void Input::SetMousePosition(const iVector2& Position)
	{
		Mouse.CurrentPosition = Position;
	}

	void Input::SetMouseButtonDown(uint32 Button, uint8 Clicks)
	{
		Mouse.ButtonsDown[Button].State = true;
		Mouse.ButtonsDown[Button].Clicks = Clicks;
	}

	void Input::SetMouseButtonUp(uint32 Button, uint8 Clicks)
	{
		Mouse.ButtonsUp[Button].State = true;
		Mouse.ButtonsUp[Button].Clicks = Clicks;
	}

	void Input::SetMouseWheel(const iVector2& State)
	{
		Mouse.Wheel = State;
	}

	void Input::SetGamepadAxis(uint32 Axis, float Value)
	{
		Gamepad.Axes[Axis] = Value;
	}

	void Input::SetGamepadButtonDown(uint32 Button)
	{
		Gamepad.Buttons[Button] = true;
		Gamepad.ButtonsDown[Button] = true;
	}

	void Input::SetGamepadButtonUp(uint32 Button)
	{
		Gamepad.Buttons[Button] = false;
		Gamepad.ButtonsUp[Button] = true;
	}
	
	bool Input::GetKey(uint32 Key) const
	{
		return Keyboard.Keys[Key] && KeyboardFocus;
	}
	
	bool Input::GetKeyDown(uint32 Key) const
	{
		return Keyboard.KeysDown[Key] && KeyboardFocus;
	}
	
	bool Input::GetKeyUp(uint32 Key) const
	{
		return Keyboard.KeysUp[Key] && KeyboardFocus;
	}

	Input::MouseButton Input::GetMouseButton(uint32 Button) const
	{
		return Mouse.Buttons[Button];
	}

	Input::MouseButton Input::GetMouseButtonDown(uint32 Button) const
	{
		return Mouse.ButtonsDown[Button];
	}

	Input::MouseButton Input::GetMouseButtonUp(uint32 Button) const
	{
		return Mouse.ButtonsUp[Button];
	}

	iVector2 Input::GetMousePosition() const
	{
		return Mouse.CurrentPosition;
	}

	iVector2 Input::GetMouseMovement() const
	{
		return Mouse.CurrentPosition - Mouse.PreviousPosition;
	}

	iVector2 Input::GetMouseWheel() const
	{
		return Mouse.Wheel;
	}

	float Input::GetGamepadAxis(GamepadAxis Axis) const
	{
		float Ax = Gamepad.Axes[static_cast<uint8>(Axis)];
		return fabs(Ax) > GamepadDead ? Ax : 0.0f;
	}

	Vector2 Input::GetGamepadStick(GamepadStick Stick) const
	{
		switch (Stick)
		{
		case GamepadStick::Left:  return { GetGamepadAxis(GamepadAxis::LStickX), GetGamepadAxis(GamepadAxis::LStickY) }; break;
		case GamepadStick::Right: return { GetGamepadAxis(GamepadAxis::RStickX), GetGamepadAxis(GamepadAxis::RStickY) }; break;
		}

		return {};
	}

	bool Input::GetGamepadButton(GamepadButton Button) const
	{
		return Gamepad.Buttons[static_cast<uint8>(Button)];
	}

	bool Input::GetGamepadButtonDown(GamepadButton Button) const
	{
		return Gamepad.ButtonsDown[static_cast<uint8>(Button)];
	}

	bool Input::GetGamepadButtonUp(GamepadButton Button) const
	{
		return Gamepad.ButtonsUp[static_cast<uint8>(Button)];
	}
	
	Input::~Input()
	{
		if (Gamepad.Internal != nullptr)
		{
			SDL_GameControllerClose((SDL_GameController*)Gamepad.Internal);
		}
	}

}


