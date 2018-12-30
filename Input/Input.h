#pragma once

#include <Math/Vector2.h>
#include <Common/Cursor/Cursor.h>
#include <SDL.h>

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
	private:
		uint8* KeyboardState = nullptr;
		int KeysNum = 0;

		bool Keys[256];
		bool KeysDown[256];
		bool KeysUp[256];
		MouseButton Buttons[8];
		MouseWheel Wheel;

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

		bool GetKey(uint32 Key) const;
		bool GetKeyDown(uint32 Key) const;
		bool GetKeyUp(uint32 Key) const;
		iVector2 GetMousePosition() const;
		iVector2 GetMouseMovement() const;
		MouseButton GetMouseButton(uint32 Button) const;
		MouseWheel GetMouseWheel() const;

		~Input();
	};

}


