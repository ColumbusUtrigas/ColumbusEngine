#include <System/Log.h>
#include <Core/Platform/Platform.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Columbus
{

	static char* LogText(const char* Fmt, const char* Text, const char* AColor, const char* BColor)
	{
		#ifdef COLUMBUS_PLATFORM_WINDOWS
			int MaxLength = strlen(Text) + strlen(Fmt) + 2;
			char* Result = (char*)malloc(MaxLength);
			Result[0] = '\0';

			strcat(Result, Text);
			strcat(Result, Fmt);
		#else
			int MaxLength = strlen(AColor) + strlen(Text) + strlen(BColor) + strlen(Fmt) + strlen("\x1b[0m") + 2;
			char* Result = (char*)malloc(MaxLength);
			Result[0] = '\0';

			strcat(Result, AColor);
			strcat(Result, Text);
			strcat(Result, BColor);
			strcat(Result, Fmt);
			strcat(Result, "\x1b[0m");
		#endif

		return strcat(Result, "\n");
	}

#define Log(Text, AColor, BColor) \
	va_list Args; \
	va_start(Args, Fmt); \
	char* Result = LogText(Fmt, Text, AColor, BColor); \
	vprintf(Result, Args); \
	free(Result); \
	va_end(Args);
	
	void Log::Initialization(const char* Fmt, ...)
	{
		Log("[INITIALIZATION]", "\x1b[36;1m", "\x1b[37;1m");
	}
	
	void Log::Success(const char* Fmt, ...)
	{
		Log("[SUCCESS]: ", "\x1b[32;1m", "\x1b[37;1m");
	}
	
	void Log::Message(const char* Fmt, ...)
	{
		Log("[INFO]: ", "\x1b[37;1m", "\x1b[37;1m");
	}
	
	void Log::Warning(const char* Fmt, ...)
	{
		Log("[WARNING]: ", "\x1b[33;1m", "\x1b[37;1m");
	}
	
	void Log::Error(const char* Fmt, ...)
	{
		Log("[ERROR]: ", "\x1b[31;1m", "\x1b[37;1m");
	}
	
	void Log::Fatal(const char* Fmt, ...)
	{
		Log("[FATAL]: ", "\x1b[35;1m", "\x1b[37;1m");
		exit(1);
	}

}


