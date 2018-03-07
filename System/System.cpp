/************************************************
*                  System.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <System/System.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	std::string GetSystem()
	{
		#ifdef COLUMBUS_PLATFORM_WINDOWS
				return "Windows";
		#endif

		#ifdef __LINUX__
				return "Linux";
		#endif

		#ifdef __unix
				return "Unix";
		#endif

		#ifdef __posix
				return "POSIX";
		#endif

		#ifdef __APPLE__
				return "Apple";
		#endif
	}
	//////////////////////////////////////////////////////////////////////////////
	bool CheckWindows()
	{
		if (GetSystem() == "Windows") return true;
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	char* C_ReadFile(const char* aPath)
	{
		FILE* file = fopen(aPath, "rt");
		if (file == NULL) return NULL;

		fseek(file, 0, SEEK_END);
		unsigned long lenth = ftell(file);
		char* data = new char[lenth + 1];
		memset(data, 0, lenth + 1);
		fseek(file, 0, SEEK_SET);
		fread(data, 1, lenth, file);
		fclose(file);
		return data;
	}

}
