#pragma once

namespace Columbus
{

	class Log
	{
	public:
		static void Initialization(const char* Fmt, ...);
		static void Success(const char* Fmt, ...);
		static void Message(const char* Fmt, ...);
		static void Warning(const char* Fmt, ...);
		static void Error(const char* Fmt, ...);
		static void Fatal(const char* Fmt, ...);
	};

}





