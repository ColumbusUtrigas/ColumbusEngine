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
		#ifdef COLUMBUS_SYSTEM_WINDOWS
				return "Windows";
		#endif

		#ifdef COLUMBUS_SYSTEM_LINUX
				return "Linux";
		#endif

		#ifdef COLUMBUS_SYSTEM_APPLE
				return "Apple";
		#endif
	}
	//////////////////////////////////////////////////////////////////////////////
	bool CheckWindows()
	{
		if (GetSystem() == "Windows") return true;
		return false;
	}

}
