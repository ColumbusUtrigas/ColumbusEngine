#pragma once

#include <cstdint>
#include <stddef.h>

namespace Columbus
{

	typedef int8_t   int8;
	typedef uint8_t  uint8;
	typedef int16_t  int16;
	typedef uint16_t uint16;
	typedef int32_t  int32;
	typedef uint32_t uint32;
	typedef int64_t  int64;
	typedef uint64_t uint64;
	typedef unsigned uint;

	typedef  int8_t  i8;
	typedef uint8_t  u8;
	typedef  int16_t i16;
	typedef uint16_t u16;
	typedef  int32_t i32;
	typedef uint32_t u32;
	typedef  int64_t i64;
	typedef uint64_t u64;
	typedef float    f32;
	typedef double   f64;

	#define IMPLEMENT_ENUM_CLASS_BITOPS(type) \
		inline constexpr bool operator!=(type x, u64 y) { return static_cast<u64>(x) != y; } \
		inline constexpr type operator|(type x, type y) { return static_cast<type>(static_cast<u64>(x) | static_cast<u64>(y)); } \
		inline constexpr type operator&(type x, type y) { return static_cast<type>(static_cast<u64>(x) & static_cast<u64>(y)); }

}
