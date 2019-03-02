#pragma once

#include <Math/Vector2.h>
#include <Common/Cursor/Cursor.h>

namespace Columbus
{

	enum class SystemCursor
	{
		Arrow,
		IBeam,
		Wait,
		Crosshair,
		WaitArrow,
		SizeNWSE,
		SizeNESW,
		SizeWE,
		SizeNS,
		SizeAll,
		No,
		Hand
	};

	class Input
	{
	public:
		struct MouseButton
		{
			int X = 0;
			int Y = 0;
			bool Pressed = false;
			uint8 Clicks = 0;
		};

		struct MouseWheel
		{
			int X = 0;
			int Y = 0;
		};

		struct GamepadAxis
		{
		private:
			uint8 Value = 0;
		public:
			enum Enum
			{
				LStickX  = 0,
				LStickY  = 1,
				RStickX  = 2,
				RStickY  = 3,
				LTrigger = 4,
				RTrigger = 5
			};
		
			GamepadAxis() {}
			GamepadAxis(uint8 Base) : Value(Base) {}
			GamepadAxis(Enum Base) : Value(Base) {}
			GamepadAxis(const GamepadAxis& Other) : Value(Other.Value) {}

			GamepadAxis& operator=(uint8 Other) { Value = Other; return *this; }
			GamepadAxis& operator=(Enum Other) { Value = Other; return *this; }
			GamepadAxis& operator=(GamepadAxis Other) { Value = Other.Value; return *this; }
			
			operator uint8() const { return Value; }
		};

		struct GamepadButton
		{
		private:
			uint8 Value = 0;
		public:
			enum
			{
				A         = 0,
				B         = 1,
				X         = 2,
				Y         = 3,
				Back      = 4,
				Guide     = 5,
				Start     = 6,
				LStick    = 7,
				RStick    = 8,
				LShoulder = 9,
				RShoulder = 10,
				DPadUp    = 11,
				DPadDown  = 12,
				DPadLeft  = 13,
				DPadRight = 14
			};

			GamepadButton() {}
			GamepadButton(uint8 Base) : Value(Base) {}
			GamepadButton(const GamepadButton& Other) : Value(Other.Value) {}

			GamepadButton& operator=(uint8 Other) { Value = Other; return *this; }
			GamepadButton& operator=(GamepadButton Other) { Value = Other.Value; return *this; }

			operator uint8() const { return Value; }
		};

		enum class Stick : uint8
		{
			Left,
			Right
		};
		
		struct Gamepad
		{
			float Axes[8] = { 0.0f };
			bool Buttons[16] = { false };
			void* Internal = nullptr;
		};
	private:
		static constexpr int MaxKeys = 512;
		static constexpr int MaxButtons = 8;
		static constexpr int MaxGamepads = 4;
		static constexpr float GamepadDead = 0.05f;

		uint8* KeyboardState = nullptr;
		int KeysNum = 0;

		bool Keys[MaxKeys];
		bool KeysDown[MaxKeys];
		bool KeysUp[MaxKeys];

		MouseButton Buttons[MaxButtons];
		MouseWheel Wheel;
		Gamepad Gamepad;

		iVector2 CurrentMousePosition;
		iVector2 PreviousMousePosition;

		bool MouseEnabled = true;
		bool KeyboardFocus;
		bool MouseFocus;
		bool KeyRepeat;
	public:
		Input();

		void ShowMouseCursor(bool Show);
		void SetCursor(Cursor InCursor);
		void SetSystemCursor(SystemCursor Cursor);
		void SetColoredCursor(const void* Pixels, uint32 Width, uint32 Height, uint32 BPP, const iVector2& Hot);

		void WarpMouse(const iVector2& aPos);

		void SetKeyboardFocus(bool Focus);
		void SetMouseFocus(bool Focus);

		void Update();

		void SetKeyDown(uint32 Key);
		void SetKeyUp(uint32 Key);
		void SetMousePosition(const iVector2& Position);
		void SetMouseButton(uint32 Button, const MouseButton& State);
		void SetMouseWheel(const MouseWheel& State);
		void SetGamepadAxis(GamepadAxis, float Value);
		void SetGamepadButton(GamepadButton Button, bool Value);

		bool GetKey(uint32 Key) const;
		bool GetKeyDown(uint32 Key) const;
		bool GetKeyUp(uint32 Key) const;

		iVector2 GetMousePosition() const;
		iVector2 GetMouseMovement() const;
		MouseButton GetMouseButton(uint32 Button) const;
		MouseWheel GetMouseWheel() const;
		float GetGamepadAxis(GamepadAxis Axis) const;
		Vector2 GetGamepadStick(Stick Stick) const;
		bool GetGamepadButton(GamepadButton Button) const;

		~Input();
	};

}


