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
	//Return current Operating System
	std::string C_GetSystem()
	{
		#ifdef _WIN32
				return "Win32";
		#endif

		#ifdef _WIN64
				return "Win64";
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
	//Check the current system, if Win32 or Win64, returns true
	bool C_CheckWindows()
	{
		if (C_GetSystem() == "Win32" || C_GetSystem() == "Win64")
			return true;
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Read file
	char* C_ReadFile(const char* aPath)
	{
		FILE* file = fopen(aPath, "rt");
		if (file == NULL)
			return NULL;

		fseek(file, 0, SEEK_END);
		unsigned long lenght = ftell(file);
		char* data = new char[lenght + 1];
		memset(data, 0, lenght + 1);
		fseek(file, 0, SEEK_SET);
		fread(data, 1, lenght, file);
		fclose(file);
		return data;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return file size
	unsigned long int C_FileSize(const char* aFile)
	{
		FILE* fp = fopen(aFile, "rb");
		if (fp == NULL)
		{
			//C_Error("Can't get size of '%s' file", aFile);
			return 0;
		}
		fseek(fp, 0, SEEK_END);
		unsigned long int l = ftell(fp);
		fclose(fp);
		return l;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Write buffer to file
	bool C_WriteFile(const char* aFile, const char* aText)
	{
			FILE* fp = fopen(aFile, "wt");
		  if (fp == NULL)
			{
  			return false;
			}

			size_t size = strlen(aText);

			for (size_t i = 0; i < size; i++)
			{
				fputc(aText[i], fp);
			}

			fclose(fp);

			return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Create file
	bool C_CreateFile(const char* aPath)
	{
		FILE* fp = fopen(aPath, "w");
		if (fp == NULL)
			return false;
		fclose(fp);
		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Delete file
	bool C_DeleteFile(std::string aPath)
	{
		int r; //Result var
		r = remove(aPath.c_str());
		if (r == 0)
			return true;
		else
			return false;
	}
	//Create folder
	bool C_CreateFolder(const char* aPath)
	{
		int r; //Result var
		#ifdef _WIN32
			//r = _mkdir(aPath); //WIN32 Folder Creating

			LPCWSTR str;
			const size_t cSize = strlen(aPath) + 1;
			wchar_t* wc = new wchar_t[cSize];
			mbstowcs(wc, aPath, cSize);
			str = wc;


			//LPSECURITY_ATTRIBUTES atr = 4555;
			//r = CreateDirectory(str, atr);
		#else
			r = mkdir(aPath, 4555); //UNIX Folder Creating
		#endif
		if (r == 0)
			return true;
		else
			return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Conversion from degrees to radians
	float C_DegToRads(float aDeg)
	{
		return aDeg * 3.141592659 / 180.0f;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Conversion from radians to degrees
	float C_RadsToDeg(float aRads)
	{
		return aRads * 180.0f / 3.141592659;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Random between two floats
	float C_RandomBetween(float aMin, float aMax)
	{
		return (aMin + (float)(rand()) / ((float)(RAND_MAX / (aMax - aMin))));
	}

}
