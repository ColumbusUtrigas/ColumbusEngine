/************************************************
*              	 EventSystem.cpp                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Impl/EventSystem.h>

namespace Columbus
{

	/*#ifdef C_SDL
		EventSystem::EventSystem()
		{
			update();
		}

		void EventSystem::update()
		{
			text = NULL;

			while(SDL_PollEvent(&mEvent))
			{
				mouseWheelLeft = false;
				mouseWheelRight = false;
				mouseWheelUp = false;
				mouseWheelDown = false;
				if(mEvent.type == SDL_KEYDOWN)
				{
					key = mEvent.key.keysym.sym;
				}

				if(mEvent.type == SDL_KEYUP)
				{
					SDL_Scancode asd;
					key = asd;
				}

				if(mEvent.type == SDL_TEXTINPUT)
				{
					text = mEvent.text.text;
				}

				if(mEvent.type == SDL_TEXTEDITING)
				{
				}

				if(mEvent.type == SDL_MOUSEBUTTONDOWN)
				{
					if(mEvent.button.button == SDL_BUTTON_LEFT)
						mouseLeft = true;
					if(mEvent.button.button == SDL_BUTTON_RIGHT)
						mouseRight = true;
					if(mEvent.button.button == SDL_BUTTON_MIDDLE)
						mouseMiddle = true;
				}

				if(mEvent.type == SDL_MOUSEBUTTONUP)
				{
					if(mEvent.button.button == SDL_BUTTON_LEFT)
						mouseLeft = false;
					if(mEvent.button.button == SDL_BUTTON_RIGHT)
						mouseRight = false;
					if(mEvent.button.button == SDL_BUTTON_MIDDLE)
						mouseMiddle = false;
				}

				if(mEvent.type == SDL_MOUSEWHEEL)
				{
					printf("wheel\n");
					if(mEvent.wheel.y > 0)
						mouseWheelUp = true;
					if(mEvent.wheel.y < 0)
						mouseWheelDown = true;
					if(mEvent.wheel.x > 0)
						mouseWheelRight = true;
					if(mEvent.wheel.x < 0)
						mouseWheelLeft = true;
				}
			}
		}

		void EventSystem::mouseCoords(int *aX, int* aY)
		{
			SDL_GetMouseState(aX, aY);
		}

		void EventSystem::globalMouseCoords(int *aX, int* aY)
		{
			SDL_GetGlobalMouseState(aX, aY);
		}

		void EventSystem::mouseRel(int *aX, int* aY)
		{
			SDL_GetRelativeMouseState(aX, aY);
		}

		bool EventSystem::isKey(int aKey)
		{
			keys = (Uint8*)SDL_GetKeyboardState(NULL);
			return keys[aKey];
		}

		bool EventSystem::isKeyDown(int aKey)
		{

		}

		bool EventSystem::isKeyUp(int aKey)
		{

		}

		bool EventSystem::isText(char aText)
		{
			if(text != NULL)
				if(text[0] == aText)
				{
					return true;
				}
			return false;
		}

		bool EventSystem::isMouseButton(int aButton)
		{
			if(aButton == SDL_BUTTON_LEFT || aButton == C_BUTTON_LEFT)
				return mouseLeft;
			if(aButton == SDL_BUTTON_RIGHT || aButton == C_BUTTON_RIGHT)
				return mouseRight;
			if(aButton == SDL_BUTTON_MIDDLE || aButton == C_BUTTON_MIDDLE)
				return mouseMiddle;
		}

		bool EventSystem::isMouseButtonClick(int aButton, int aClicks, int *aX, int* aY)
		{
			SDL_PollEvent(&mEvent);
			if(mEvent.type == SDL_MOUSEBUTTONDOWN)
				if(mEvent.button.button == aButton)
					if(mEvent.button.clicks == aClicks)
					{
						mEvent.button.clicks = 0;
						SDL_GetMouseState(aX, aY);
						return true;
					}
			return false;
		}

		bool EventSystem::isExit()
		{
			return (mEvent.type == SDL_QUIT);
		}

		char EventSystem::getText()
		{
			if(text != NULL)
				return text[0];
		}

		int EventSystem::getWheelX()
		{
			if(mouseWheelLeft == true)
				return -1;
			if(mouseWheelRight == true)
				return 1;
			return 0;
		}

		int EventSystem::getWheelY()
		{
			if(mouseWheelDown == true)
				return -1;
			if(mouseWheelUp == true)
				return 1;
			return 0;
		}

		EventSystem::~EventSystem()
		{

		}
	#endif*/

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	EventSystem::EventSystem()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//Poll all events
	void EventSystem::pollEvents()
	{
		while(SDL_PollEvent(&mEvent))
		{
			if(mEvent.type == SDL_QUIT)
				mQuit = true;

			for(auto i : mWindows)
				i->pollEvent(mEvent);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Add window
	void EventSystem::addWindow(SDLWindow* aWindow)
	{
		mWindows.push_back(aWindow);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return running
	bool EventSystem::isRun()
	{
		return !mQuit;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	EventSystem::~EventSystem()
	{

	}

}
