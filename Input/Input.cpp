#include <Input/Input.h>
#include <SDL.h>

namespace Columbus
{

	Input::Input()
	{
		Keyboard.KeyboardState = (uint8*)SDL_GetKeyboardState(&Keyboard.KeysNum);
	}
	
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

		for (int i = 0; i < Keyboard.KeysNum && i < DeviceKeyboard::MaxKeys; i++) Keyboard.Keys[i] = Keyboard.KeyboardState[i] != 0;
		memset(Keyboard.KeysDown, 0, sizeof(Keyboard.KeysDown));
		memset(Keyboard.KeysUp, 0, sizeof(Keyboard.KeysUp));

		for (int i = 0; i < DeviceMouse::MaxButtons; i++) Mouse.Buttons[i].State = (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(i)) != 0;
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

	void Input::PollEvent(const Event& E)
	{
		switch (E.Type)
		{
		default: break;
		case Event::Type_Key:
			if (!E.Key.Repeat)
			{
				if ( E.Key.Pressed) Keyboard.KeysDown[E.Key.Code] = true;
				if (!E.Key.Pressed) Keyboard.KeysUp  [E.Key.Code] = true;
			}
			break;

		case Event::Type_Mouse:
			Mouse.CurrentPosition = { E.Mouse.X, E.Mouse.Y };
			break;

		case Event::Type_MouseButton:
			if (E.MouseButton.Pressed)
			{
				Mouse.ButtonsDown[E.MouseButton.Code].State = true;
				Mouse.ButtonsDown[E.MouseButton.Code].Clicks = E.MouseButton.Clicks;
			} else
			{
				Mouse.ButtonsUp[E.MouseButton.Code].State = true;
				Mouse.ButtonsUp[E.MouseButton.Code].Clicks = E.MouseButton.Clicks;
			}
			break;

		case Event::Type_MouseWheel:
			Mouse.Wheel = { E.MouseWheel.X, E.MouseWheel.Y };
			break;

		case Event::Type_ControllerAxis:
		{
			if (E.ControllerAxis.Controller != nullptr)
			{
				int Index = GetGamepadIndexByName(SDL_JoystickName(SDL_GameControllerGetJoystick((SDL_GameController*)E.ControllerAxis.Controller)));
				if (Index != -1)
				{
					if (E.ControllerAxis.Controller == Gamepads[Index].Internal)
					{
						Gamepads[Index].Axes[E.ControllerAxis.Code] = E.ControllerAxis.Value;
					}
				}
			}
			break;
		}

		case Event::Type_ControllerButton:
		{
			if (E.ControllerButton.Controller != nullptr)
			{
				int Index = GetGamepadIndexByName(SDL_JoystickName(SDL_GameControllerGetJoystick((SDL_GameController*)E.ControllerButton.Controller)));
				if (Index != -1)
				{
					if (E.ControllerButton.Controller == Gamepads[Index].Internal)
					{
						if (E.ControllerButton.Pressed)
						{
							Gamepads[Index].Buttons[E.ControllerButton.Code] = true;
							Gamepads[Index].ButtonsDown[E.ControllerButton.Code] = true;
						} else
						{
							Gamepads[Index].Buttons[E.ControllerButton.Code] = false;
							Gamepads[Index].ButtonsUp[E.ControllerButton.Code] = true;
						}
					}
				}
			}

			break;
		}

		case Event::Type_ControllerDevice:
		{
			switch (E.ControllerDevice.Type)
			{
			case ControllerDeviceEvent::Type_None: break;
			case ControllerDeviceEvent::Type_Added:
			{
				const char* JName = SDL_JoystickName(SDL_GameControllerGetJoystick((SDL_GameController*)E.ControllerDevice.Controller));

				if (E.ControllerDevice.Controller != nullptr && JName != nullptr)
				{
					// Check if controller is currently in the list
					bool Exit = false;
					for (int i = 0; i < MaxGamepads; i++)
					{
						if (Gamepads[i].Name != nullptr)
						{
							if (strcmp(Gamepads[i].Name, JName) == 0)
							{
								Exit = true;
								break;
							}
						}
					}
					if (Exit) break;

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
						Gamepads[FreeID].Internal = E.ControllerDevice.Controller;
						Gamepads[FreeID].Name = JName;
					}
				}

				break;
			}

			case ControllerDeviceEvent::Type_Removed:
			{
				if (E.ControllerDevice.Controller != nullptr)
				{
					int Index = GetGamepadIndexByName(SDL_JoystickName(SDL_GameControllerGetJoystick((SDL_GameController*)E.ControllerDevice.Controller)));
					if (Index != -1)
					{
						if (Gamepads[Index].Internal != nullptr)
						{
							Gamepads[Index] = DeviceGamepad();
						}
					}
				}

				break;
			}
			}
		}
		}
	}

	void Input::SetMousePosition(const iVector2& Position)
	{
		Mouse.CurrentPosition = Position;
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

		//printf("suka\n");

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


