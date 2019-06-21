#include <Input/Input.h>
#include <SDL.h>

namespace Columbus
{

	Input::Input() {}
	
	void Input::ShowMouseCursor(bool Show)
	{
		MouseEnabled = Show;
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

	bool Input::IsMouseCursorShowed() const
	{
		return MouseEnabled;
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

		memset(Keyboard.KeysDown, 0, sizeof(Keyboard.KeysDown));
		memset(Keyboard.KeysUp, 0, sizeof(Keyboard.KeysUp));

		memset(Mouse.ButtonsDown, 0, sizeof(Mouse.ButtonsDown));
		memset(Mouse.ButtonsUp, 0, sizeof(Mouse.ButtonsUp));

		for (int i = 0; i < MaxGamepads; i++)
		{
			memset(Gamepads[i].ButtonsDown, 0, sizeof(Gamepads[i].ButtonsDown));
			memset(Gamepads[i].ButtonsUp, 0, sizeof(Gamepads[i].ButtonsUp));
		}

		Mouse.PreviousPosition = Mouse.CurrentPosition;
		Mouse.Wheel = { 0, 0 };

		SDL_ShowCursor(MouseEnabled ? SDL_ENABLE : SDL_DISABLE);
	}

	void Input::PollKeyEvent(const KeyEvent& E)
	{
		if (E.Repeat)
		{
			Keyboard.KeysRepeated[E.Code] = E.Pressed;
		} else
		{
			Keyboard.Keys[E.Code] = E.Pressed;
			Keyboard.KeysRepeated[E.Code] = false;

			if ( E.Pressed) Keyboard.KeysDown[E.Code] = true;
			if (!E.Pressed) Keyboard.KeysUp  [E.Code] = true;
		} 
	}

	void Input::PollMouseEvent(const MouseEvent& E)
	{
		Mouse.CurrentPosition = { E.X, E.Y };
	}

	void Input::PollMouseButtonEvent(const MouseButtonEvent& E)
	{
		Mouse.Buttons[E.Code].State = E.Pressed;

		if (E.Pressed)
		{
			Mouse.ButtonsDown[E.Code].State = true;
			Mouse.ButtonsDown[E.Code].Clicks = E.Clicks;
		} else
		{
			Mouse.ButtonsUp[E.Code].State = true;
			Mouse.ButtonsUp[E.Code].Clicks = E.Clicks;
		}
	}

	void Input::PollMouseWheelEvent(const MouseWheelEvent& E)
	{
		Mouse.Wheel = { E.X, E.Y };
	}

	void Input::PollControllerAxisEvent(const ControllerAxisEvent& E)
	{
		if (E.Controller != nullptr)
		{
			int Index = GetGamepadIndexByName(SDL_JoystickName(SDL_GameControllerGetJoystick((SDL_GameController*)E.Controller)));
			if (Index != -1)
			{
				if (E.Controller == Gamepads[Index].Internal)
				{
					Gamepads[Index].Axes[E.Code] = E.Value;
				}
			}
		}
	}

	void Input::PollControllerButtonEvent(const ControllerButtonEvent& E)
	{
		if (E.Controller != nullptr)
		{
			int Index = GetGamepadIndexByName(SDL_JoystickName(SDL_GameControllerGetJoystick((SDL_GameController*)E.Controller)));
			if (Index != -1)
			{
				if (E.Controller == Gamepads[Index].Internal)
				{
					if (E.Pressed)
					{
						Gamepads[Index].Buttons[E.Code] = true;
						Gamepads[Index].ButtonsDown[E.Code] = true;
					} else
					{
						Gamepads[Index].Buttons[E.Code] = false;
						Gamepads[Index].ButtonsUp[E.Code] = true;
					}
				}
			}
		}
	}

	void Input::ControllerAdded(const ControllerDeviceEvent& E)
	{
		const char* JName = SDL_JoystickName(SDL_GameControllerGetJoystick((SDL_GameController*)E.Controller));

		if (E.Controller != nullptr && JName != nullptr)
		{
			// Check if controller is currently in the list
			for (int i = 0; i < MaxGamepads; i++)
			{
				if (Gamepads[i].Name != nullptr)
				{
					if (strcmp(Gamepads[i].Name, JName) == 0)
					{
						return;
					}
				}
			}

			int FreeID = -1;
			for (int i = 0; i < MaxGamepads; i++)
			{
				if (Gamepads[i].Internal == nullptr)
				{
					FreeID = i;
					break;
				}
			}

			if (FreeID >= 0 && FreeID < MaxGamepads)
			{
				Gamepads[FreeID].Internal = E.Controller;
				Gamepads[FreeID].Name = JName;
			}
		}
	}

	void Input::ControllerRemoved(const ControllerDeviceEvent& E)
	{
		if (E.Controller != nullptr)
		{
			int Index = GetGamepadIndexByName(SDL_JoystickName(SDL_GameControllerGetJoystick((SDL_GameController*)E.Controller)));
			if (Index != -1)
			{
				if (Gamepads[Index].Internal != nullptr)
				{
					Gamepads[Index] = DeviceGamepad();
				}
			}
		}
	}

	void Input::PollControllerDeviceEvent(const ControllerDeviceEvent& E)
	{
		switch (E.Type)
		{
		case ControllerDeviceEvent::Type_None: break;
		case ControllerDeviceEvent::Type_Added: ControllerAdded(E); break;
		case ControllerDeviceEvent::Type_Removed: ControllerRemoved(E); break;
		}
	}

	void Input::PollEvent(const Event& E)
	{
		switch (E.Type)
		{
		default: break;
		case Event::Type_Key: PollKeyEvent(E.Key); break;
		case Event::Type_Mouse: PollMouseEvent(E.Mouse); break;
		case Event::Type_MouseButton: PollMouseButtonEvent(E.MouseButton); break;
		case Event::Type_MouseWheel: PollMouseWheelEvent(E.MouseWheel); break;
		case Event::Type_ControllerAxis: PollControllerAxisEvent(E.ControllerAxis); break;
		case Event::Type_ControllerButton: PollControllerButtonEvent(E.ControllerButton); break;
		case Event::Type_ControllerDevice: PollControllerDeviceEvent(E.ControllerDevice); break;
		}
	}

	void Input::SetMousePosition(const iVector2& Position)
	{
		Mouse.CurrentPosition = Position;
	}

	bool Input::GetKey(uint32 Key, bool Repeat) const
	{
		return (Repeat ? Keyboard.KeysRepeated[Key] : Keyboard.Keys[Key]) && KeyboardFocus;
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

	int Input::GetGamepadIndexByName(const char* Name) const
	{
		for (int i = 0; i < MaxGamepads; i++)
		{
			if (Gamepads[i].Name != nullptr)
			{
				if (strcmp(Gamepads[i].Name, Name) == 0)
				{
					return i;
				}
			}
		}

		return -1;
	}

	bool Input::IsGamepadAttached(int Index) const
	{
		if (Index < 0 || Index >= MaxGamepads) return false;
		if (Gamepads[Index].Internal == nullptr) return false;

		return SDL_JoystickGetAttached(SDL_GameControllerGetJoystick((SDL_GameController*)Gamepads[Index].Internal)) == SDL_TRUE;
	}

	float Input::GetGamepadAxis(uint32 Index, GamepadAxis Axis) const
	{
		float Ax = Gamepads[Index].Axes[static_cast<uint8>(Axis)];
		return fabs(Ax) > GamepadDead ? Ax : 0.0f;
	}

	Vector2 Input::GetGamepadStick(uint32 Index, GamepadStick Stick) const
	{
		switch (Stick)
		{
		case GamepadStick::Left:  return { GetGamepadAxis(Index, GamepadAxis::LStickX), GetGamepadAxis(Index, GamepadAxis::LStickY) }; break;
		case GamepadStick::Right: return { GetGamepadAxis(Index, GamepadAxis::RStickX), GetGamepadAxis(Index, GamepadAxis::RStickY) }; break;
		}

		return {};
	}

	bool Input::GetGamepadButton(uint32 Index, GamepadButton Button) const
	{
		return Gamepads[Index].Buttons[static_cast<uint8>(Button)];
	}

	bool Input::GetGamepadButtonDown(uint32 Index, GamepadButton Button) const
	{
		return Gamepads[Index].ButtonsDown[static_cast<uint8>(Button)];
	}

	bool Input::GetGamepadButtonUp(uint32 Index, GamepadButton Button) const
	{
		return Gamepads[Index].ButtonsUp[static_cast<uint8>(Button)];
	}

	const char* Input::GetGamepadName(uint32 Index) const
	{
		return Gamepads[Index].Name;
	}
	
	Input::~Input()
	{
		for (int i = 0; i < MaxGamepads; i++)
		{
			if (Gamepads[i].Internal != nullptr)
			{
				SDL_GameControllerClose((SDL_GameController*)Gamepads[i].Internal);
			}
		}
	}

}


