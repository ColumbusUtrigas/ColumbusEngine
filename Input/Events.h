#pragma once

#include <Core/Types.h>

namespace Columbus
{

	struct KeyEvent
	{
		uint32 Code;
		bool Pressed;
		bool Repeat;
	};

	struct MouseEvent
	{
		int32 X, Y;
	};

	struct MouseButtonEvent
	{
		uint32 Code;
		uint32 Clicks;
		bool Pressed;
	};

	struct MouseWheelEvent
	{
		float X, Y;
	};

	struct ControllerAxisEvent
	{
		uint32 Code;
		float Value;
		void* Controller;
	};

	struct ControllerButtonEvent
	{
		uint32 Code;
		bool Pressed;
		void* Controller;
	};

	struct ControllerDeviceEvent
	{
		enum
		{
			Type_None,
			Type_Added,
			Type_Removed
		} Type = Type_None;

		void* Controller;
	};

	struct WindowEvent
	{
		enum
		{
			Type_None,

			Type_Close,
			Type_Shown,
			Type_Hidden,
			Type_Minimized,
			Type_Maximized,
			Type_MouseEnter,
			Type_MouseLeave,
			Type_KeyboardFocusGained,
			Type_KeyboardFocusLost,
			Type_Resize
		} Type = Type_None;

		int32 Data1, Data2;
		void* Window;
	};

	struct Event
	{
		enum
		{
			Type_None,
			Type_Quit,
			Type_Key,
			Type_Mouse,
			Type_MouseButton,
			Type_MouseWheel,
			Type_ControllerAxis,
			Type_ControllerButton,
			Type_ControllerDevice,
			Type_Window
		} Type = Type_None;

		KeyEvent Key;
		MouseEvent Mouse;
		MouseButtonEvent MouseButton;
		MouseWheelEvent MouseWheel;
		ControllerAxisEvent ControllerAxis;
		ControllerButtonEvent ControllerButton;
		ControllerDeviceEvent ControllerDevice;
		WindowEvent Window;
	};

}


