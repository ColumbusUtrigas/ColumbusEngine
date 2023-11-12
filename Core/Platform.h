#pragma once

namespace Columbus
{

	bool IsRunningWithDebugger();

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
		#define DEBUGBREAK if (IsRunningWithDebugger()) __debugbreak
	#elif defined(__GNUC__)
		#define COMPILER_GCC 1
		#define DEBUGBREAK if (IsRunningWithDebugger()) __builtin_trap
	#elif defined(__clang__)
		#define COMPILER_CLANG 1
		#define DEBUGBREAK if (IsRunningWithDebugger()) __builtin_trap
	#else
		#error Unsupported compiler.
	#endif

	#if defined(PLATFORM_WINDOWS)
		#define FORCEINLINE __forceinline
	#else
		#define FORCEINLINE __attribute__((always_inline))
	#endif

}


