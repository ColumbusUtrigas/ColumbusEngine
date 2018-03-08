/************************************************
*                   System.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <iostream>
#include <tinyxml2.h>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/stat.h>
#endif

#undef COLUMBUS_SYSTEM_WINDOWS
#undef COLUMBUS_SYSTEM_LINUX
#undef COLUMBUS_SYSTEM_APPLE

#undef COLUMBUS_INLINE

#if defined(_WIN32)
	#define COLUMBUS_SYSTEM_WINDOWS
#elif defined(_WIN64)
	#define COLUMBUS_SYSTEM_WINDOWS
#elif defined(__linux)
	#define COLUMBUS_SYSTEM_LINUX
#elif defined(__APPLE__)
	#define COLUMBUS_SYSTEM_APPLE
#else
	#error Unsupported platform
#endif

#ifdef COLUMBUS_SYSTEM_WINDOWS
	#define COLUMBUS_INLINE __forceinline
#else
	#define COLUMBUS_INLINE inline
#endif

typedef std::string stdstr;
typedef int C_Int;
typedef int cint;
typedef tinyxml2::XMLDocument C_XMLDoc;
typedef tinyxml2::XMLNode C_XMLNode;
typedef tinyxml2::XMLElement C_XMLElement;
#define C_XML_SUCCESS tinyxml2::XML_SUCCESS

namespace Columbus
{

	std::string GetSystem();
	bool CheckWindows();

}
