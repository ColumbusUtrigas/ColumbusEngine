#pragma once

#include <tinyxml2.h>

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

typedef tinyxml2::XMLDocument C_XMLDoc;
typedef tinyxml2::XMLNode C_XMLNode;
typedef tinyxml2::XMLElement C_XMLElement;
#define C_XML_SUCCESS tinyxml2::XML_SUCCESS

namespace Columbus
{

	const char* GetSystem();
	bool CheckWindows();

}
