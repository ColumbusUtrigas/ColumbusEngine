#include <System/System.h>

namespace C
{

	char* C_GetSystem()
	{
		#ifdef _WIN32
				return (char*)"Win32";
		#endif

		#ifdef _WIN64
				return (char*)"Win64";
		#endif

		#ifdef __LINUX__
				return (char*)"Linux";
		#endif

		#ifdef __unix
				return (char*)"Unix";
		#endif

		#ifdef __posix
				return (char*)"POSIX";
		#endif

		#ifdef __APPLE__
				return (char*)"Apple";
		#endif
	}

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

	bool C_CreateFile(const char* aPath)
	{
		FILE* fp = fopen(aPath, "w");
		if (fp == NULL)
			return false;
		fclose(fp);
		return true;
	}

	bool C_DeleteFile(const char* aPath)
	{
		int r; //Result var
		r = remove(aPath);
		if (r == 0)
			return true;
		else
			return false;
	}

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

	float C_DegToRads(float aDeg)
	{
		return aDeg * 3.141592659 / 180.0f;
	}

	float C_RadsToDeg(float aRads)
	{
		return aRads * 180.0f / 3.141592659;
	}


}



