/************************************************
*                     Log.h                     *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   05.11.2017                  *
*************************************************/

#pragma once

#include <cstdio>
#include <cstdarg>
#include <string>

#include <System/System.h>

namespace Columbus
{

	class C_Log
	{
	public:
		static void initialization(const char* aFmt, ...);
		static void success(const char* aFmt, ...);
		static void message(const char* aFmt, ...);
		static void warning(const char* aFmt, ...);
		static void error(const char* aFmt, ...);
		static void fatal(const char* aFmt, ...);

		static void initialization(std::string aMessage);
		static void success(std::string aMessage);
		static void message(std::string aMessage);
		static void warning(std::string aMessage);
		static void error(std::string aMessage);
		static void fatal(std::string aMessage);
	};

}





