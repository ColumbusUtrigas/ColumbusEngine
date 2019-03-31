#include <Input/EventSystem.h>
#include <SDL2/SDL.h>

namespace Columbus
{

	void EventSystem::Update()
	{
		SDL_Event E;

		Event internal;

		while (SDL_PollEvent(&E))
		{
			RawFunction(&E);

			switch (E.type)
			{
			default: return; break;
			case SDL_QUIT: internal.Type = Event::Type_Quit; break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				internal.Type = Event::Type_Key;
				internal.Key.Code = E.key.keysym.scancode;
				internal.Key.Pressed = (E.key.state == SDL_PRESSED);
				internal.Key.Repeat = (E.key.repeat != 0);
				break;

			case SDL_MOUSEMOTION:
				internal.Type = Event::Type_Mouse;
				internal.Mouse.X = E.motion.x;
				internal.Mouse.Y = E.motion.y;
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				internal.Type = Event::Type_MouseButton;
				internal.MouseButton.Code = E.button.button;
				internal.MouseButton.Pressed = (E.button.state == SDL_PRESSED);
				internal.MouseButton.Clicks = E.button.clicks;
				break;

			case SDL_MOUSEWHEEL:
				internal.Type = Event::Type_MouseWheel;
				internal.MouseWheel.X = E.wheel.x;
				internal.MouseWheel.Y = E.wheel.y;
				break;

			case SDL_CONTROLLERAXISMOTION:
				internal.Type = Event::Type_ControllerAxis;
				internal.ControllerAxis.Code = E.caxis.axis;
				internal.ControllerAxis.Value = (float)(E.caxis.value) / 32768.0f;
				internal.ControllerAxis.Controller = SDL_GameControllerFromInstanceID(E.caxis.which);
				break;

			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				internal.Type = Event::Type_ControllerButton;
				internal.ControllerButton.Code = E.cbutton.button;
				internal.ControllerButton.Pressed = (E.cbutton.state == SDL_PRESSED);
				internal.ControllerButton.Controller = SDL_GameControllerFromInstanceID(E.cbutton.which);
				break;

			case SDL_CONTROLLERDEVICEADDED:
				internal.Type = Event::Type_ControllerDevice;
				internal.ControllerDevice.Type = ControllerDeviceEvent::Type_Added;
				internal.ControllerDevice.Controller = SDL_GameControllerOpen(E.cdevice.which);
				break;

			case SDL_CONTROLLERDEVICEREMOVED:
				internal.Type = Event::Type_ControllerDevice;
				internal.ControllerDevice.Type = ControllerDeviceEvent::Type_Removed;
				internal.ControllerDevice.Controller = SDL_GameControllerFromInstanceID(E.cdevice.which);
				break;

			case SDL_WINDOWEVENT:
			{
				internal.Type = Event::Type_Window;
				internal.Window.Data1 = E.window.data1;
				internal.Window.Data2 = E.window.data2;
				internal.Window.Window = SDL_GetWindowFromID(E.window.windowID);

				switch (E.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					internal.Window.Type = WindowEvent::Type_Close;
					break;
				case SDL_WINDOWEVENT_SHOWN:
					internal.Window.Type = WindowEvent::Type_Shown;
					break;
				case SDL_WINDOWEVENT_HIDDEN:
					internal.Window.Type = WindowEvent::Type_Hidden;
					break;
				case SDL_WINDOWEVENT_MINIMIZED:
					internal.Window.Type = WindowEvent::Type_Minimized;
					break;
				case SDL_WINDOWEVENT_MAXIMIZED:
					internal.Window.Type = WindowEvent::Type_Maximized;
					break;
				case SDL_WINDOWEVENT_ENTER:
					internal.Window.Type = WindowEvent::Type_MouseEnter;
					break;
				case SDL_WINDOWEVENT_LEAVE:
					internal.Window.Type = WindowEvent::Type_MouseLeave;
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					internal.Window.Type = WindowEvent::Type_KeyboardFocusGained;
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					internal.Window.Type = WindowEvent::Type_KeyboardFocusLost;
					break;
				case SDL_WINDOWEVENT_RESIZED:
					internal.Window.Type = WindowEvent::Type_Resize;
					break;
				}
				break;
			}
			}

			switch (internal.Type)
			{
			case Event::Type_None: break;
			case Event::Type_Quit: QuitFunction(internal); break;
			case Event::Type_Key:
			case Event::Type_Mouse:
			case Event::Type_MouseButton:
			case Event::Type_MouseWheel:
			case Event::Type_ControllerAxis:
			case Event::Type_ControllerButton:
			case Event::Type_ControllerDevice:
				InputFunction(internal);
				break;
			case Event::Type_Window: WindowFunction(internal); break;
			}
		}
	}

}


