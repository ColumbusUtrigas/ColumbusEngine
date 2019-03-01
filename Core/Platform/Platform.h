#pragma once

namespace Columbus
{

	#undef COLUMBUS_PLATFORM_WINDOWS
	#undef COLUMBUS_PLATFORM_LINUX
	#undef COLUMBUS_PLATFORM_APPLE
	#undef FORCEINLINE
	#undef USE_SIMD_EXTENSIONS

	#define USE_SIMD_EXTENSIONS 1

	#if defined(_WIN32)
		#define COLUMBUS_PLATFORM_WINDOWS
		#define COLUMBUS_PLATFORM COLUMBUS_PLATFORM_WINDOWS
	#elif defined(_WIN64)
		#define COLUMBUS_PLATFORM_WINDOWS
		#define COLUMBUS_PLATFORM COLUMBUS_PLATFORM_WINDOWS
	#elif defined(__linux)
		#define COLUMBUS_PLATFORM_LINUX
		#define COLUMBUS_PLATFORM COLUMBUS_PLATFORM_LINUX
	#elif defined(__APPLE__)
		#define COLUMBUS_PLATFORM_APPLE
		#define COLUMBUS_PLATFORM COLUMBUS_PLATFORM_APPLE
	#else
		#error Unsupported platform
	#endif

	#if defined(COLUMBUS_PLATFORM_WINDOWS)
		#define FORCEINLINE __forceinline
	#else
		#define FORCEINLINE inline
	#endif

}


