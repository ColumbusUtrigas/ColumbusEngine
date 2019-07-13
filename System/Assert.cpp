#include <System/Assert.h>
#include <Core/Platform/Platform.h>
#include <string.h>
#include <stdio.h>

#ifdef PLATFORM_WINDOWS
	#error "assert is not implemented"
#else
	#include <SDL.h>
	#define MESSAGE_BOX(message) \
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, \
			"Fatal Error", \
			message, \
			NULL);
#endif

namespace Columbus
{

	void _ColumbusAssert_Internal(const char* file, long int line, const char* msg)
	{
		char line_str[16] = { 0 };
		sprintf(line_str, "%ld", line);

		const char* file_str = strstr(file, "ColumbusEngine");

		char message[4096] = { 0 };

		strcat(message, "Assertation at ");
		strcat(message, file_str);
		strcat(message, ":");
		strcat(message, line_str);
		strcat(message, "\n\n");
		strcat(message, msg);

		MESSAGE_BOX(message);

		Log::Fatal("Assertation at %s:%d:   %s", file_str, line, msg);
	}

}


