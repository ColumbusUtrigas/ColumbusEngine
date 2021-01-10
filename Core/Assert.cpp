#include <Core/Assert.h>
#include <Core/Platform.h>
#include <string.h>
#include <stdio.h>

#ifdef PLATFORM_WINDOWS
	#include <windows.h>
	#define MESSAGE_BOX(message) MessageBox(NULL, message, "Fatal Error", MB_APPLMODAL | MB_ICONERROR | MB_OK);
#else
	#include <SDL.h>
	#define MESSAGE_BOX(message) \
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, \
			"Fatal Error", \
			message, \
			NULL);
#endif

void _ColumbusAssert_Internal(const char* file, long int line, const char* msg)
{
	constexpr int buf_size = 4096;
	char message[buf_size] = { 0 };
	snprintf(message, buf_size, "Assertation at %s:%li\n\n%s", file, line, msg);

	MESSAGE_BOX(message);

	Columbus::Log::Fatal("Assertation at %s:%d:   %s", file, line, msg);
}

