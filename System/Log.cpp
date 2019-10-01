#include <System/Log.h>
#include <Core/Platform.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PLATFORM_WINDOWS
	#include <windows.h>
#endif

namespace Columbus
{

	std::vector<Log::Msg> g_LogBuffer;

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

#define AddToBuffer(Type) \
	Msg m; \
	m.type = Type; \
	m.text = msg; \
	g_LogBuffer.push_back(m);

#define Log(Text, AColor, BColor, Type) \
	va_list Args, ArgsCopy; \
	va_start(Args, Fmt); \
	va_copy(ArgsCopy, Args); \
	SetConsoleColor(AColor); \
	printf(Text); \
	SetConsoleColor(BColor); \
	auto size = vsnprintf(nullptr, 0, Fmt, Args) + 1; \
	std::string msg(size, ' '); \
	vsnprintf(&msg.front(), size, Fmt, ArgsCopy); \
	printf("%s\n", msg.c_str()); \
	AddToBuffer(Type); \
	va_end(Args);
	
	void Log::Initialization(const char* Fmt, ...)
	{
		Log("[INITIALIZATION]: ", Color_Cyan, Color_White, Type_Initialization);
	}
	
	void Log::Success(const char* Fmt, ...)
	{
		Log("[SUCCESS]: ", Color_Green, Color_White, Type_Success);
	}
	
	void Log::Message(const char* Fmt, ...)
	{
		Log("[INFO]: ", Color_White, Color_White, Type_Message);
	}
	
	void Log::Warning(const char* Fmt, ...)
	{
		Log("[WARNING]: ", Color_Yellow, Color_White, Type_Warning);
	}
	
	void Log::Error(const char* Fmt, ...)
	{
		Log("[ERROR]: ", Color_Red, Color_White, Type_Error);
	}
	
	void Log::Fatal(const char* Fmt, ...)
	{
		Log("[FATAL]: ", Color_Purple, Color_White, Type_Fatal);
		exit(1);
	}

	std::vector<Log::Msg>& Log::GetData()
	{
		return g_LogBuffer;
	}

}


