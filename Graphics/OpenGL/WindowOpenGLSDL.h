#pragma once

#include <Graphics/Window.h>
#include <SDL.h>

namespace Columbus
{

	class WindowOpenGLSDL : public Window
	{
	private:
		SDL_Window* mWindow = nullptr;
		SDL_GLContext mGLC;
		SDL_Event mTmpEvent;

		void initializeSDL();
		void initializeWindow(const Vector2 aSize, const std::string aTitle, const WindowFlags aFlags);
		void initializeOpenGL();
		void getVersions();
	public:
		WindowOpenGLSDL();
		WindowOpenGLSDL(const Vector2 aSize, const std::string aTitle, const WindowFlags aFlags);

		bool create(const Vector2 aSize, const std::string aTitle, const WindowFlags aFlags) override;

		void update() override;
		void clear(const Vector4 aColor) override;
		void display() override;

		void setVSync(const bool aVSync) override;
		void setSize(const Vector2 aSize) override;

		SDL_Window* getHandle() const;
		void pollEvent(SDL_Event& aEvent);
		std::string getType() const override;

		~WindowOpenGLSDL();
	};

}








