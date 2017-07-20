#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <tinyxml2.h>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/stat.h>
#endif

typedef const char* C_Str;
typedef const char* cstr;
typedef int C_Int;
typedef int cint;
typedef tinyxml2::XMLDocument C_XMLDoc;
typedef tinyxml2::XMLNode C_XMLNode;
typedef tinyxml2::XMLElement C_XMLElement;
#define C_XML_SUCCESS tinyxml2::XML_SUCCESS

namespace C
{

	char* C_GetSystem();

	char* C_ReadFile(const char* aPath);

	unsigned long int C_FileSize(const char* aFile);

	bool C_WriteFile(const char* aFile, const char* aText);

	bool C_CreateFile(const char* aPath);

	bool C_DeleteFile(const char* aPath);

	bool C_CreateFolder(const char* aPath);

	float C_DegToRads(float aDeg);

	float C_RadsToDeg(float aRads);


}
