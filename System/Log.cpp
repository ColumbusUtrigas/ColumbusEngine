#include <System/Log.h>
#include <Core/Platform/Platform.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PLATFORM_WINDOWS
	#include <Windows.h>
#endif

namespace Columbus
{

	enum Color
	{
		Color_White,
		Color_Cyan,
		Color_Green,
		Color_Yellow,
		Color_Red,
		Color_Purple
	};

	static void SetConsoleColor(Color C)
	{
		#ifdef PLATFORM_WINDOWS
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

			int Mode = 0;

			switch (C)
			{
			case Color_White: Mode = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN; break;
			case Color_Cyan: Mode = FOREGROUND_BLUE | FOREGROUND_GREEN; break;
			case Color_Green: Mode = FOREGROUND_GREEN; break;
			case Color_Yellow: Mode = FOREGROUND_RED | FOREGROUND_GREEN; break;
			case Color_Red: Mode = FOREGROUND_RED; break;
			case Color_Purple: Mode = FOREGROUND_RED | FOREGROUND_BLUE; break;
			}

			SetConsoleTextAttribute(hConsole, Mode | FOREGROUND_INTENSITY);
		#else
			switch (C)
			{
			case Color_White:  printf("\x1b[37;1m"); break;
			case Color_Cyan:   printf("\x1b[36;1m"); break;
			case Color_Green:  printf("\x1b[32;1m"); break;
			case Color_Yellow: printf("\x1b[33;1m"); break;
			case Color_Red:    printf("\x1b[31;1m"); break;
			case Color_Purple: printf("\x1b[35;1m"); break;
			}
		#endif
	}

#define Log(Text, AColor, BColor) \
	va_list Args; \
	va_start(Args, Fmt); \
	SetConsoleColor(AColor); \
	printf(Text); \
	SetConsoleColor(BColor); \
	vprintf(Fmt, Args); \
	printf("\n"); \
	va_end(Args);
	
	void Log::Initialization(const char* Fmt, ...)
	{
		Log("[INITIALIZATION]: ", Color_Cyan, Color_White);
	}
	
	void Log::Success(const char* Fmt, ...)
	{
		Log("[SUCCESS]: ", Color_Green, Color_White);
	}
	
	void Log::Message(const char* Fmt, ...)
	{
		Log("[INFO]: ", Color_White, Color_White);
	}
	
	void Log::Warning(const char* Fmt, ...)
	{
		Log("[WARNING]: ", Color_Yellow, Color_White);
	}
	
	void Log::Error(const char* Fmt, ...)
	{
		Log("[ERROR]: ", Color_Red, Color_White);
	}
	
	void Log::Fatal(const char* Fmt, ...)
	{
		Log("[FATAL]: ", Color_Purple, Color_White);
		exit(1);
	}

}


