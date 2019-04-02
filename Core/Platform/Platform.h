#pragma once

namespace Columbus
{

	#define USE_SIMD_EXTENSIONS 1

	#if defined(_WIN32)
		#define PLATFORM_WINDOWS 1
	#elif defined(__linux)
		#define PLATFORM_LINUX 1
	#else
		#error Unsupported platform.
	#endif

	#if defined(_MSC_VER)
		#define COMPLLER_MSVC 1
	#elif defined(__GNUC__)
		#define COMPILER_GCC 1
	#elif defined(__clang__)
		#define COMPILER_CLANG 1
	#else
		#error Unsupported compiler.
	#endif

	#if defined(COLUMBUS_PLATFORM_WINDOWS)
		#define FORCEINLINE __forceinline
	#else
		#define FORCEINLINE __attribute__((always_inline))
	#endif

}


