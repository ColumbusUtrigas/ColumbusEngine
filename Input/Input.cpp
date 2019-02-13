#include <Input/Input.h>
#include <SDL.h>

namespace Columbus
{

	Input::Input()
	{
		KeyboardState = (uint8_t*)SDL_GetKeyboardState(&KeysNum);

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
		case SystemCursor::Arrow:     ID = SDL_SYSTEM_CURSOR_ARROW; break;
		case SystemCursor::IBeam:     ID = SDL_SYSTEM_CURSOR_IBEAM; break;
		case SystemCursor::Wait:      ID = SDL_SYSTEM_CURSOR_WAIT; break;
		case SystemCursor::Crosshair: ID = SDL_SYSTEM_CURSOR_CROSSHAIR; break;
		case SystemCursor::WaitArrow: ID = SDL_SYSTEM_CURSOR_WAITARROW; break;
		case SystemCursor::SizeNWSE:  ID = SDL_SYSTEM_CURSOR_SIZENWSE; break;
		case SystemCursor::SizeNESW:  ID = SDL_SYSTEM_CURSOR_SIZENESW; break;
		case SystemCursor::SizeWE:    ID = SDL_SYSTEM_CURSOR_SIZEWE; break;
		case SystemCursor::SizeNS:    ID = SDL_SYSTEM_CURSOR_SIZENS; break;
		case SystemCursor::SizeAll:   ID = SDL_SYSTEM_CURSOR_SIZEALL; break;
		case SystemCursor::No:        ID = SDL_SYSTEM_CURSOR_NO; break;
		case SystemCursor::Hand:      ID = SDL_SYSTEM_CURSOR_HAND; break;
		default:                      ID = SDL_SYSTEM_CURSOR_NO; break;
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
		CurrentMousePosition  = Pos;
		PreviousMousePosition = Pos;
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

		for (int i = 0; i < KeysNum; i++) Keys[i] = KeyboardState[i] != 0;
		memset(KeysDown, 0, sizeof(KeysDown));
		memset(KeysUp, 0, sizeof(KeysUp));
		memset(Buttons, 0, sizeof(Buttons));

		PreviousMousePosition = CurrentMousePosition;
		Wheel = { 0, 0 };
	}

	void Input::SetKeyDown(uint32 Key)
	{
		KeysDown[Key] = true;
	}

	void Input::SetKeyUp(uint32 Key)
	{
		KeysUp[Key] = true;
	}

	void Input::SetMousePosition(const iVector2& Position)
	{
		CurrentMousePosition = Position;
	}

	void Input::SetMouseButton(uint32 Button, const Input::MouseButton& State)
	{
		Buttons[Button] = State;
	}

	void Input::SetMouseWheel(const MouseWheel& State)
	{
		Wheel = State;
	}

	void Input::SetGamepadAxis(GamepadAxis Axis, float Value)
	{
		Gamepad.Axes[Axis] = Value;
	}

	void Input::SetGamepadButton(GamepadButton Button, bool Value)
	{
		Gamepad.Buttons[Button] = Value;
	}
	
	bool Input::GetKey(uint32 Key) const
	{
		return Keys[Key] && KeyboardFocus;
	}
	
	bool Input::GetKeyDown(uint32 Key) const
	{
		return KeysDown[Key] && KeyboardFocus;
	}
	
	bool Input::GetKeyUp(uint32 Key) const
	{
		return KeysUp[Key] && KeyboardFocus;
	}

	iVector2 Input::GetMousePosition() const
	{
		return CurrentMousePosition;
	}

	iVector2 Input::GetMouseMovement() const
	{
		return CurrentMousePosition - PreviousMousePosition;
	}

	Input::MouseButton Input::GetMouseButton(uint32 Button) const
	{
		return Buttons[Button];
	}

	Input::MouseWheel Input::GetMouseWheel() const
	{
		return Wheel;
	}

	float Input::GetGamepadAxis(GamepadAxis Axis) const
	{
		return fabs(Gamepad.Axes[Axis]) > GamepadDead ? Gamepad.Axes[Axis] : 0.0f;
	}

	Vector2 Input::GetGamepadStick(Stick Stick) const
	{
		switch (Stick)
		{
		case Stick::Left:  return { GetGamepadAxis(GamepadAxis::LStickX), GetGamepadAxis(GamepadAxis::LStickY) }; break;
		case Stick::Right: return { GetGamepadAxis(GamepadAxis::RStickX), GetGamepadAxis(GamepadAxis::RStickY) }; break;
		}

		return {};
	}

	bool Input::GetGamepadButton(GamepadButton Button) const
	{
		return Gamepad.Buttons[Button];
	}
	
	Input::~Input()
	{
		if (Gamepad.Internal != nullptr)
		{
			SDL_GameControllerClose((SDL_GameController*)Gamepad.Internal);
		}
	}

}


