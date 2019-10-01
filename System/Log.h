#pragma once

#include <vector>
#include <string>

namespace Columbus
{

	class Log
	{
	public:
		enum Type
		{
			Type_Initialization,
			Type_Success,
			Type_Message,
			Type_Warning,
			Type_Error,
			Type_Fatal
		};

		struct Msg
		{
			std::string text;
			Type type;
		};
	public:
		static void Initialization(const char* Fmt, ...);
		static void Success(const char* Fmt, ...);
		static void Message(const char* Fmt, ...);
		static void Warning(const char* Fmt, ...);
		static void Error(const char* Fmt, ...);
		static void Fatal(const char* Fmt, ...);

		static std::vector<Msg>& GetData();
	};

}


