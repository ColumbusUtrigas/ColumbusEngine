/************************************************
*                    Log.cpp                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   05.11.2017                  *
*************************************************/

#include <System/Log.h>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	//Generates system-depended text
	static std::string LogText(const char* aFmt, const char* aPreText,
		const char* aPreColor, const char* aColor)
	{
		std::string pre = aPreText;
		std::string fmt;

		if (CheckWindows())
		{
			fmt = pre + aFmt;
		} else
		{
			fmt = aPreColor + pre + aColor + aFmt;
		}

		if (!CheckWindows())
		{
			fmt += "\x1b[0m";
		}
		
		return fmt + "\n";
	}
	//////////////////////////////////////////////////////////////////////////////
	void Log::initialization(const char* aFmt, ...)
	{
		std::string fmt = LogText(aFmt, "[INITIALIZATION]: ", "\x1b[36;1m", "\x1b[37;1m");

		va_list args;
		va_start(args, aFmt);
		vprintf(fmt.c_str(), args);
		va_end(args);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Log::success(const char* aFmt, ...)
	{
		std::string fmt = LogText(aFmt, "[SUCCESS]: ", "\x1b[32;1m", "\x1b[37;1m");

		va_list args;
		va_start(args, aFmt);
		vprintf(fmt.c_str(), args);
		va_end(args);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Log::message(const char* aFmt, ...)
	{
		std::string fmt = LogText(aFmt, "[INFO]: ", "\x1b[37;1m", "\x1b[37;1m");

		va_list args;
		va_start(args, aFmt);
		vprintf(fmt.c_str(), args);
		va_end(args);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Log::warning(const char* aFmt, ...)
	{
		std::string fmt = LogText(aFmt, "[WARNING]: ", "\x1b[33;1m", "\x1b[37;1m");

		va_list args;
		va_start(args, aFmt);
		vprintf(fmt.c_str(), args);
		va_end(args);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Log::error(const char* aFmt, ...)
	{
		std::string fmt = LogText(aFmt, "[ERROR]: ", "\x1b[33;1m", "\x1b[37;1m");

		va_list args;
		va_start(args, aFmt);
		vprintf(fmt.c_str(), args);
		va_end(args);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Log::fatal(const char* aFmt, ...)
	{
		std::string fmt = LogText(aFmt, "[FATAL]: ", "\x1b[35;1m", "\x1b[37;1m");

		va_list args;
		va_start(args, aFmt);
		vprintf(fmt.c_str(), args);
		va_end(args);

		exit(1);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void Log::initialization(std::string aMessage)
	{
		std::string mes = LogText(aMessage.c_str(), "[INITIALIZATION]: ", "\x1b[36;1m", "\x1b[37;1m");

		printf("%s", mes.c_str());
	}
	//////////////////////////////////////////////////////////////////////////////
	void Log::success(std::string aMessage)
	{
		std::string mes = LogText(aMessage.c_str(), "[SUCCESS]: ", "\x1b[32;1m", "\x1b[37;1m");

		printf("%s", mes.c_str());
	}
	//////////////////////////////////////////////////////////////////////////////
	void Log::message(std::string aMessage)
	{
		std::string mes = LogText(aMessage.c_str(), "[INFO]: ", "\x1b[37;1m", "\x1b[37;1m");

		printf("%s", mes.c_str());
	}
	//////////////////////////////////////////////////////////////////////////////
	void Log::warning(std::string aMessage)
	{
		std::string mes = LogText(aMessage.c_str(), "[WARNING]: ", "\x1b[33;1m", "\x1b[37;1m");

		printf("%s", mes.c_str());
	}
	//////////////////////////////////////////////////////////////////////////////
	void Log::error(std::string aMessage)
	{
		std::string mes = LogText(aMessage.c_str(), "[ERROR]: ", "\x1b[31;1m", "\x1b[37;1m");

		printf("%s", mes.c_str());
	}
	//////////////////////////////////////////////////////////////////////////////
	void Log::fatal(std::string aMessage)
	{
		std::string mes = LogText(aMessage.c_str(), "[FATAL]: ", "\x1b[35;1m", "\x1b[37;1m");

		printf("%s", mes.c_str());
		exit(1);
	}


}


