#pragma once

#include <Graphics/Window.h>
#include <SDL.h>

namespace Columbus
{

	class WindowOpenGLSDL : public Window
	{
	private:
		SDL_Window* Window;
		SDL_GLContext Context;
		SDL_Event TmpEvent;

		void InitializeSDL();
		void InitializeWindow(const iVector2& InSize, const char* Title, Window::Flags F);
		void InitializeOpenGL();
		void GetVersions();
	public:
		WindowOpenGLSDL();
		WindowOpenGLSDL(const iVector2& InSize, const char* Title, Window::Flags F);

		virtual bool Create(const iVector2& InSize, const char* Title, Window::Flags F) final override;
		virtual void Close() final override;

		virtual void Update() final override;
		virtual void Clear(const Vector4& Color) final override;
		virtual void Display() final override;

		virtual void SetSize(const iVector2& Size) final override;
		virtual void SetMousePosition(const iVector2& Pos) final override;

		virtual void PollEvent(const Event& E) final override;

		void PollEvent(SDL_Event& Event);

		SDL_Window* GetWindowHandle() const;
		SDL_GLContext GetContextHandle() const;

		virtual ~WindowOpenGLSDL() final override;
	};

}


