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

	#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || \
		defined(__i386) || defined(__i486__) || defined(__i486) ||  \
		defined(i386) || defined(__ia64__) || defined(__x86_64__)
		#define COLUMBUS_X86_OR_X64_CPU 1
	#else
		#define COLUMBUS_X86_OR_X64_CPU 0
	#endif

	#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || COLUMBUS_X86_OR_X64_CPU
		#define COLUMBUS_LITTLE_ENDIAN 1
	#else
		#define COLUMBUS_LITTLE_ENDIAN 0
	#endif

}
