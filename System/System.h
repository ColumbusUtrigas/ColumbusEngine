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

#ifdef _WIN32
	#define COLUMBUS_SYSTEM_WINDOWS
#elif _WIN64
	#define COLUMBUS_SYSTEM_WINDOWS
#elif __LINUX__
	#define COLUMBUS_SYSTEM_LINUX
#elif __APPLE__
	#define COLUMBUS_SYSTEM_APPLE
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

	//Return current Operating System
	std::string C_GetSystem();
	//Check the current system, if Win32 or Win64, returns true
	bool C_CheckWindows();
	//Read file
	char* C_ReadFile(const char* aPath);
	//Return file size
	unsigned long int C_FileSize(const char* aFile);
	//Write buffer to file
	bool C_WriteFile(const char* aFile, const char* aText);
	//Create file
	bool C_CreateFile(const char* aPath);
	//Delete file
	bool C_DeleteFile(std::string aPath);
	//Create folder
	bool C_CreateFolder(const char* aPath);
	//Conversion from degrees to radians
	float C_DegToRads(const float aDeg);
	//Conversion from radians to degrees
	float C_RadsToDeg(const float aRads);
	//Random between two floats
	float C_RandomBetween(float aMin, float aMax);


}
