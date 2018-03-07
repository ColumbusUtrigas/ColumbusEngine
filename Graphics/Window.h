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
		private:
			bool mVSync = true;
			bool mOpen = true;
			bool mKeyFocus = true;
			bool mMouseFocus = true;
			bool mShown = false;
			bool mMinimised = false;

			float mRedrawTime = 0.0;
			unsigned int mFPS = 0;

			std::string mTitle;

			Vector2 mSize = Vector2(640, 480);
		public:
			Window(const Vector2 aSize, const std::string aTitle, const WindowFlags aFlags);

			void update();
			void clear(const Vector4 aColor);
			void display();

			void setVSync(const bool aVSync);
			bool getVSync() const;

			void setSize(const Vector2 aSize);
			Vector2 getSize() const;
			float getAspect() const;
			float getRedrawTime() const;
			unsigned int getFPS() const;

			bool isOpen() const;
			bool isKeyFocus() const;
			bool isMouseFocus() const;
			bool isShown() const;
			bool isMinimised() const;

			~Window();
	};

}












