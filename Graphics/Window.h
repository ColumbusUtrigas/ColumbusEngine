/************************************************
*                    Window.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   16.01.2018                  *
*************************************************/
#pragma once

#include <Math/Vector2.h>
#include <Math/Vector4.h>
#include <System/System.h>
#include <System/Timer.h>

namespace Columbus
{

	enum WindowFlags
	{
		E_WINDOW_FLAG_RESIZABLE,
		E_WINDOW_FLAG_FULLSCREEN,
		E_WINDOW_FLAG_FULLSCREEN_DESKTOP
	};

	class Window
	{
	protected:
		bool mVSync = true;
		bool mOpen = true;
		bool mKeyFocus = true;
		bool mMouseFocus = true;
		bool mShown = false;
		bool mMinimized = false;
		bool mMaximized = false;
		bool mInitialized = false;

		float mRedrawTime = 0.0f;
		unsigned int mFPS = 0;
		unsigned int mFrames = 0;

		Timer mRedrawTimer;
		Timer mFPSTimer;

		std::string mTitle;

		Vector2 mSize = Vector2(640, 480);
	public:
		Window();
		Window(const Vector2 aSize, const std::string aTitle, const WindowFlags aFlags);

		virtual bool create(const Vector2 aSize, const std::string aTitle, const WindowFlags aFlags);

		virtual void update();
		virtual void clear(const Vector4 aColor);
		virtual void display();

		virtual void setVSync(const bool aVSync);
		virtual bool getVSync() const;

		virtual void setSize(const Vector2 aSize);
		virtual Vector2 getSize() const;
		virtual float getAspect() const;
		virtual float getRedrawTime() const;
		virtual unsigned int getFPS() const;

		virtual bool isOpen() const;
		virtual bool isKeyFocus() const;
		virtual bool isMouseFocus() const;
		virtual bool isShown() const;
		virtual bool isMinimised() const;

		virtual std::string getType() const;

		~Window();
	};

}












