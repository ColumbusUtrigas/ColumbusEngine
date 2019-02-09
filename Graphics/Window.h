#pragma once

#include <Math/Vector2.h>
#include <Math/Vector4.h>
#include <System/Timer.h>

namespace Columbus
{

	class Window
	{
	public:
		enum class Flags
		{
			Resizable  = 1 << 0,
			Fullscreen = 1 << 1
		};
	protected:
		bool VSync = true;
		bool Open = true;
		bool KeyFocus = true;
		bool MouseFocus = true;
		bool Shown = false;
		bool Minimized = false;
		bool Maximized = false;
		bool Initialized = false;

		float RedrawTime = 0.0f;
		uint32 FPS = 0;
		uint32 Frames = 0;

		Timer RedrawTimer;
		Timer FPSTimer;

		iVector2 Size = iVector2(640, 480);
	public:
		Window() {}
		Window(const iVector2& InSize, const char* Title, Flags F) {}

		virtual bool Create(const iVector2& InSize, const char* Title, Flags F) = 0;
		virtual void Close() { Open = false; }

		virtual void Update() = 0;
		virtual void Clear(const Vector4& Color) = 0;
		virtual void Display() = 0;

		virtual void SetVSync(bool NewVSync) { VSync = NewVSync; }

		virtual void SetSize(const iVector2& NewSize) { Size = NewSize; }
		virtual iVector2 GetSize() const { return Size; }
		virtual float GetAspect() const { return (float)Size.X / (float)Size.Y; }
		virtual float GetRedrawTime() const { return RedrawTime; }
		virtual uint32 GetFPS() const { return FPS; }

		virtual void SetMousePosition(const iVector2& Pos) = 0;

		virtual bool IsOpen() const { return Open; }
		virtual bool HasKeyFocus() const { return KeyFocus; }
		virtual bool HasMouseFocus() const { return MouseFocus; }
		virtual bool IsShown() const { return Shown; }
		virtual bool IsMinimized() const { return Minimized; }

		virtual ~Window() {}
	};

}












