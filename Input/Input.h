#pragma once

#include <Math/Vector2.h>
#include <Common/Cursor/Cursor.h>
#include <Input/Events.h>

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
			bool State = false;
			uint32 Clicks = 0;
		};

		enum class GamepadAxis : uint8
		{
			LStickX  = 0,
			LStickY  = 1,
			RStickX  = 2,
			RStickY  = 3,
			LTrigger = 4,
			RTrigger = 5,
			Max      = 6
		};

		enum class GamepadButton : uint8
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
			DPadRight = 14,
			Max       = 15
		};

		enum class GamepadStick : uint8
		{
			Left,
			Right
		};
	private:
		struct DeviceKeyboard
		{
			static constexpr int MaxKeys = 512;

			uint8* KeyboardState = nullptr;
			int KeysNum = 0;

			bool Keys[MaxKeys];
			bool KeysDown[MaxKeys];
			bool KeysUp[MaxKeys];
		};

		struct DeviceMouse
		{
			static constexpr int MaxButtons = 8;

			MouseButton Buttons[MaxButtons];
			MouseButton ButtonsDown[MaxButtons];
			MouseButton ButtonsUp[MaxButtons];

			iVector2 CurrentPosition;
			iVector2 PreviousPosition;
			iVector2 Wheel;
		};
		
		struct DeviceGamepad
		{
			static constexpr int MaxAxes = 8;
			static constexpr int MaxButtons = 16;

			float Axes[MaxAxes] = { 0.0f };
			bool Buttons[MaxButtons] = { false };
			bool ButtonsDown[MaxButtons] = { false };
			bool ButtonsUp[MaxButtons] = { false };
			void* Internal = nullptr;

			const char* Name = nullptr;
		};
	private:
		static constexpr int MaxGamepads = 4;
		static constexpr float GamepadDead = 0.05f;

		DeviceKeyboard Keyboard;
		DeviceMouse Mouse;
		DeviceGamepad Gamepads[MaxGamepads];

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

		bool IsMouseCursorShowed() const;

		void WarpMouse(const iVector2& aPos);

		void SetKeyboardFocus(bool Focus);
		void SetMouseFocus(bool Focus);
		void SetMousePosition(const iVector2& Position);

		void Update();
		void PollEvent(const Event& E);

		bool GetKey(uint32 Key) const;
		bool GetKeyDown(uint32 Key) const;
		bool GetKeyUp(uint32 Key) const;

		MouseButton GetMouseButton(uint32 Button) const;
		MouseButton GetMouseButtonDown(uint32 Button) const;
		MouseButton GetMouseButtonUp(uint32 Button) const;

		iVector2 GetMousePosition() const;
		iVector2 GetMouseMovement() const;
		iVector2 GetMouseWheel() const;

		int GetGamepadIndexByName(const char* Name) const;
		bool IsGamepadAttached(int Index) const;

		float GetGamepadAxis(uint32 Index, GamepadAxis Axis) const;
		Vector2 GetGamepadStick(uint32 Index, GamepadStick Stick) const;
		bool GetGamepadButton(uint32 Index, GamepadButton Button) const;
		bool GetGamepadButtonDown(uint32 Index, GamepadButton Button) const;
		bool GetGamepadButtonUp(uint32 Index, GamepadButton Button) const;
		const char* GetGamepadName(uint32 Index) const;

		~Input();
	};

}


